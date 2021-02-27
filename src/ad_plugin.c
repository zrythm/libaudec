/*
 * Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
 *
 * This file is part of libaudec
 *
 * libaudec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libaudec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with libaudec.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2011-2013 Robin Gareus <robin@gareus.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <samplerate.h>

#include "ad_plugin.h"

AudecLogLevel audec_log_level =
  AUDEC_LOG_LEVEL_ERROR;

audec_log_fn_t log_fn = NULL;

#define UNUSED(x) (void)(x)
int     ad_eval_null(const char *f) { UNUSED(f); return -1; }
void *  ad_open_null(const char *f, AudecInfo *n) { UNUSED(f); UNUSED(n); return NULL; }
int     ad_close_null(void *x) { UNUSED(x); return -1; }
int     ad_info_null(void *x, AudecInfo *n) { UNUSED(x); UNUSED(n); return -1; }
int64_t ad_seek_null(void *x, int64_t p) { UNUSED(x); UNUSED(p); return -1; }
ssize_t ad_read_null(void *x, float*d, size_t s) { UNUSED(x); UNUSED(d); UNUSED(s); return -1;}


typedef struct adecoder
{
  /** Decoder backend plugin. */
  ad_plugin const * plugin;

  /** Backend data, such as SF file. */
  void *            data;

  /* Log function. */
  audec_log_fn_t    log_fn;
} adecoder;

/* samplecat api */

void audec_init() { /* global init */ }

static ad_plugin const *
choose_backend (
  const char * fn)
{
  int max, val;
  ad_plugin const * plugin = NULL;
  max = 0;

  val = adp_get_sndfile()->eval(fn);
  if (val > max)
    {
      max = val;
      plugin = adp_get_sndfile();
    }

#if 0
  val = adp_get_ffmpeg()->eval(fn);
  if (val > max)
    {
      max = val;
      plugin = adp_get_ffmpeg();
    }
#endif

  val = adp_get_minimp3()->eval(fn);
  if (val > max)
    {
      max = val;
      plugin = adp_get_minimp3();
    }

  return plugin;
}

AudecHandle *
audec_open (
  const char * filename,
  AudecInfo *  nfo)
{
  adecoder * decoder =
    calloc (1, sizeof (adecoder));
  audec_clear_nfo (nfo);

  decoder->plugin = choose_backend (filename);
  if (!decoder->plugin)
    {
      dbg (
        AUDEC_LOG_LEVEL_ERROR,
        "fatal: no decoder backend available");
      free(decoder);
      return NULL;
    }
  decoder->data = decoder->plugin->open (filename, nfo);
  if (!decoder->data)
    {
      free (decoder);
      return NULL;
    }
  return (AudecHandle *) decoder;
}

int
audec_info (
  AudecHandle * handle,
  AudecInfo *   nfo)
{
  adecoder * decoder = (adecoder *) handle;
  if (!decoder)
    return -1;
  return decoder->plugin->info (decoder->data, nfo);
}

int
audec_close (
  AudecHandle * handle)
{
  adecoder * decoder = (adecoder*) handle;
  if (!decoder)
    return -1;
  int ret = decoder->plugin->close (decoder->data);
  free (decoder);
  return ret;
}

int64_t
audec_seek (
  AudecHandle * handle,
  int64_t       pos)
{
  adecoder * decoder = (adecoder*) handle;
  if (!decoder) return -1;
  return decoder->plugin->seek (decoder->data, pos);
}

/**
 * Returns the size of the buffer that must be
 * allocated to load the file with the given
 * sample rate.
 */
static ssize_t
get_buf_size_for_sample_rate (
  AudecInfo *  nfo,
  unsigned int sample_rate)
{
  if (sample_rate == nfo->sample_rate)
    {
      return nfo->frames * nfo->channels;
    }
  else
    {
      double resample_ratio =
        (double) sample_rate / nfo->sample_rate;
      if (fabs (resample_ratio - 1.0) < 1e-20)
        {
          /* no sample rate change needed */
          return
            (ssize_t) nfo->frames *
            (ssize_t) nfo->channels;
        }
      if (src_is_valid_ratio (resample_ratio) == 0)
        {
          dbg (
            AUDEC_LOG_LEVEL_ERROR,
            "Sample rate change out of valid "
            "range.");
          return -1;
        }

      return
        (ssize_t)
        ((double) nfo->frames * resample_ratio *
         (double) nfo->channels);
    }
}

typedef struct SrcCallbackData
{
  float *    in_frames;
  size_t     num_in_frames;
} SrcCallbackData;

static long
src_cb (
  SrcCallbackData * data,
  float **          audio)
{
  *audio = &(data->in_frames[0]);

  return data->num_in_frames;
}

ssize_t
audec_read (
  AudecHandle * handle,
  float **      out,
  int           sample_rate)
{
  adecoder *decoder = (adecoder*) handle;
  if (!decoder)
    return -1;

  if (*out != NULL)
    {
      dbg (
        AUDEC_LOG_LEVEL_ERROR,
        "Please set 'out' to NULL before calling "
        "audec_read()");
      return -1;
    }

  /* get the info */
  AudecInfo nfo;
  audec_info ((AudecHandle *) decoder, &nfo);

  /* read the frames */
  size_t in_len =
    (size_t) (nfo.frames * nfo.channels);
  float * in = malloc (in_len * sizeof (float));
  ssize_t ret = /* note: includes channels */
    decoder->plugin->read (
      decoder->data, in, in_len);

  /* verify that the frames were all read */
  if (ret != (ssize_t) in_len)
    {
      dbg (
        AUDEC_LOG_LEVEL_DEBUG,
        "Number of read in frames %zu not equal to "
        "given buf size %zd",
        ret, in_len);
    }

  if (ret > (ssize_t) in_len)
    {
      dbg (
        AUDEC_LOG_LEVEL_ERROR,
        "Number of read in frames %zu greater than "
        "given buf size %zd",
        ret, in_len);
      free (in);
      return -1;
    }

  /* resample if required */
  if (sample_rate > 0 &&
      sample_rate != (int) nfo.sample_rate)
    {
      ssize_t out_buf_size =
        get_buf_size_for_sample_rate (
          &nfo, (unsigned int) sample_rate);
      if (out_buf_size < 0)
        {
          free (in);
          return -1;
        }
      else
        {
          int err;
          SrcCallbackData data = {
            .in_frames = in,
            .num_in_frames = nfo.frames,
          };
          SRC_STATE * state =
            src_callback_new (
              (src_callback_t) src_cb,
              SRC_SINC_BEST_QUALITY,
              (int) nfo.channels, &err, &data);
          if (!state)
            {
              dbg (
                AUDEC_LOG_LEVEL_ERROR,
                "Failed to create a src callback: "
                "%s", src_strerror (err));
              free (in);
              return -1;
            }

          double resample_ratio =
            (1.0 * (double) sample_rate) /
            nfo.sample_rate;
          *out =
            malloc (
              (size_t) out_buf_size *
              sizeof (float));

          /* start reading */
          ssize_t frames_read = -1;
          ssize_t total_read = 0;
          ssize_t frames_to_read = 0;
          ssize_t num_out_frames =
            (out_buf_size / (ssize_t) nfo.channels);
          do
            {
              frames_to_read =
                MIN (
                  6000,
                  num_out_frames - total_read);
              frames_read =
                src_callback_read (
                  state, resample_ratio,
                  frames_to_read,
                  &((*out)[total_read * nfo.channels]));

              /* handle errors */
              int err_ret =
                src_error (state);
              if (err_ret)
                {
                  dbg (
                    AUDEC_LOG_LEVEL_ERROR,
                    "An error occurred during "
                    "resampling: %s",
                    src_strerror (err_ret));
                  src_delete (state);
                  free (in);
                  free (*out);
                  *out = NULL;
                  return -1;
                }

              total_read += frames_read;

              if (frames_read == -1)
                break;

            } while (frames_read > 0);
          src_delete (state);
          free (in);

          if (total_read != num_out_frames)
            {
              dbg (
                AUDEC_LOG_LEVEL_INFO,
                "Total frames read (%zu) and out "
                "frames expected (%zu) do not match",
                total_read, num_out_frames);
            }
          if (frames_read == -1)
            {
              dbg (
                AUDEC_LOG_LEVEL_ERROR,
                "An error has occurred in "
                "resampling: frames read == -1");
              free (*out);
              *out = NULL;
              return -1;
            }
          ret = total_read;

          dbg (
            AUDEC_LOG_LEVEL_INFO,
            "%zu frames read after resampling "
            "(out buffer size %zu)",
            total_read, out_buf_size);
        }
    }
  else
    {
      *out = in;
      ret = nfo.frames;

      dbg (
        AUDEC_LOG_LEVEL_INFO,
        "No resampling done, returning %" PRIi64 " frames "
        "(out buffer size %zu)",
        nfo.frames, ret);
    }

  return ret;
}

/*
 *  side-effects: allocates buffer
 */
ssize_t
audec_read_mono_dbl (
  void *      sf,
  AudecInfo * nfo,
  double *    d,
  size_t      len,
  int         sample_rate)
{
  unsigned int c,f;
  unsigned int chn = nfo->channels;
  if (len<1) return 0;

  static float *buf = NULL;

  len = audec_read (sf, &buf, sample_rate);

  for (f=0;f< (len/chn);f++) {
    double val=0.0;
    for (c=0;c<chn;c++) {
      val+=buf[f*chn + c];
    }
    d[f]= val/chn;
  }
  return len/chn;
}


int
audec_finfo (
  const char * filename, AudecInfo *nfo)
{
  audec_clear_nfo (nfo);
  void * sf = audec_open (filename, nfo);
  return audec_close(sf) ? 1 : 0;
}

void
audec_clear_nfo (
  AudecInfo *nfo)
{
  memset (nfo, 0, sizeof (AudecInfo));
}

void
audec_free_nfo (
  AudecInfo *nfo)
{
  if (nfo->meta_data)
    free (nfo->meta_data);
}

void
audec_dump_info (
  AudecLogLevel dbglvl,
  AudecInfo *     nfo)
{
  dbg(dbglvl, "sample_rate: %u", nfo->sample_rate);
  dbg(dbglvl, "channels:    %u", nfo->channels);
  dbg(dbglvl, "length:      %"PRIi64" ms", nfo->length);
  dbg(dbglvl, "frames:      %"PRIi64, nfo->frames);
  dbg(dbglvl, "bit_rate:    %d", nfo->bit_rate);
  dbg(dbglvl, "bit_depth:   %d", nfo->bit_depth);
  dbg(dbglvl, "channels:    %u", nfo->channels);
  dbg(dbglvl, "meta-data:   %s", nfo->meta_data?nfo->meta_data:"-");
}

void
audec_log (
  const char *    func,
  AudecLogLevel level,
  const char *    format,
  ...)
{
  va_list args;

  va_start (args, format);
  if (log_fn)
    {
      log_fn (level, format, args);
    }
  else if (level <= audec_log_level)
    {
      fprintf (stderr, "%s(): ", func);
      vfprintf (stderr, format, args);
      fprintf (stderr, "\n");
    }
  va_end (args);
}

void
audec_set_log_func (
  audec_log_fn_t _log_fn)
{
  log_fn = _log_fn;
}

void
audec_set_log_level (
  AudecLogLevel lvl)
{
  audec_log_level = lvl;
}
