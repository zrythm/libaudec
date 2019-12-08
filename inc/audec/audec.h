/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>
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

#ifndef __AUDEC_AUDEC_H__
#define __AUDEC_AUDEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdint.h>

typedef void AudecHandle;

typedef struct AudecInfo
{
  unsigned int sample_rate;
  unsigned int channels;

  /** Milliseconds. */
  int64_t       length;

  /** Total number of frames (eg. a frame for 16bit
   * stereo is 4 bytes. */
  int64_t       frames;

  int           bit_rate;
  int           bit_depth;
  char *        meta_data;

  /** A handle used in various operations. */
  AudecHandle * handle;
} AudecInfo;

typedef enum AudecDebugLevel
{
  AUDEC_DEBUG_LEVEL_SILENT = -1,
  AUDEC_DEBUG_LEVEL_ERROR,
  AUDEC_DEBUG_LEVEL_INFO,
  AUDEC_DEBUG_LEVEL_DEBUG,
  AUDEC_DEBUG_LEVEL_TRACE,
} AudecDebugLevel;

/**
 * Global init function - register codecs.
 */
void
audec_init (void);

/* --- public API --- */

/**
 * Open an audio file.
 *
 * @param filename file-name
 * @param nfo pointer to a adinfo struct which will hold
 * information about the file.
 *
 * @return NULL on error, a pointer to an opaque
 * soundfile-decoder object on success.
 */
AudecHandle *
audec_open (
  const char * filename,
  AudecInfo *  nfo);

/**
 * Close an audio file and release decoder structures.
 *
 * @param handle Decoder handle.
 * @return 0 on succees, -1 if sf was invalid or not open (return value can usually be ignored)
 */
int
audec_close (
  AudecHandle * handle);

/**
 * Seek to a given position in the file.
 *
 * @param handle Decoder handle.
 * @param pos frame position to seek to in frames (1 frame =
 * number-of-channel samples) from the start of the file.
 *
 * @return the current position in frames (multi-channel
 * samples) from the start of the file. On error this
 * function returns -1.
 */
int64_t
audec_seek (
  AudecHandle * handle,
  int64_t       pos);

/**
 * Decode audio data chunk to raw interleaved channel
 * floating point data.
 *
 * @param handle decoder handle
 * @param out Memory location to store output data.
 * the memory this points to will be allocated. The
 * size of this array will be the return value times
 * the number of channels.
 * @param sample_rate Sample rate to resample to. If
 * this is negative or if the file's sample rate is
 * the same as the given sample rate no resampling
 * will be done.
 *
 * @return the total number of read samples for each
 * channel.
 */
ssize_t
audec_read (
  AudecHandle * handle,
  float **      out,
  int           sample_rate);

/**
 * Re-read the file information and meta-data.
 *
 * This is not neccesary in general \ref ad_open includes
 * an implicit call but meta-data may change in live-stream
 * in which case en explicit call to ad_into is needed to
 * update the inforation.
 *
 * @param fn file-name
 * @param nfo pointer to a adinfo struct which will hold
 * information about the file.
 * @return 0 on succees, -1 if sf was invalid or not open
 */
int
audec_info (
  AudecHandle * decoder_handle,
  AudecInfo *          nfo);

/**
 * Zero initialize the information struct (does not free
 * nfo->meta_data).
 *
 * @param nfo pointer to a adinfo struct
 */
void
audec_clear_nfo (
  AudecInfo * nfo);

/** free possibly allocated meta-data text
 * @param nfo pointer to a adinfo struct
 */
void
audec_free_nfo (
  AudecInfo * nfo);


/* --- helper functions --- */

/**
 * Read file info.
 *
 * Combines ad_open() and ad_close().
 */
int
audec_finfo (
  const char * filename,
  AudecInfo *  nfo);

/**
 * Wrapper around \ref audec_read, downmixes all channels to
 * mono.
 */
ssize_t
audec_read_mono_dbl (
  void *, AudecInfo *, double*, size_t, int);

/**
 * Calls dbg() to print file info to stderr.
 *
 * @param dbglvl
 * @param info
 */
void
audec_dump_info (
  AudecDebugLevel dbglvl,
  AudecInfo *     nfo);

/**
 * Set audio-decoder debug level.
 *
 * All info is printed to stderr.
 *
 * @param lvl debug-level threshold.
 */
void
audec_set_debug_level (
  AudecDebugLevel lvl);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __AUDEC_AUDEC_H__ */
