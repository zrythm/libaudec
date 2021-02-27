/*
 * Copyright (C) 2019-2021 Alexandros Theodotou <alex at zrythm dot org>
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

#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

/**
   Helper macro needed for AUDEC_SYMBOL_EXPORT when using C++.
*/
#ifdef __cplusplus
#  define AUDEC_SYMBOL_EXTERN extern "C"
#else
#  define AUDEC_SYMBOL_EXTERN
#endif

/**
   Put this (AUDEC_SYMBOL_EXPORT) before any functions that are to be loaded
   by the host as a symbol from the dynamic library.
*/
#ifdef _WIN32
#  define AUDEC_SYMBOL_EXPORT AUDEC_SYMBOL_EXTERN __declspec(dllexport)
#else
#  define AUDEC_SYMBOL_EXPORT \
    AUDEC_SYMBOL_EXTERN __attribute__((visibility("default")))
#endif

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

  /** BPM, or 0. */
  float         bpm;

  /** A handle used in various operations. */
  AudecHandle * handle;
} AudecInfo;

typedef enum AudecLogLevel
{
  AUDEC_LOG_LEVEL_SILENT = -1,
  AUDEC_LOG_LEVEL_ERROR,
  AUDEC_LOG_LEVEL_INFO,
  AUDEC_LOG_LEVEL_DEBUG,
  AUDEC_LOG_LEVEL_TRACE,
} AudecLogLevel;

/**
 * Logging function prototype.
 */
typedef void (*audec_log_fn_t)(
  AudecLogLevel level,
  const char *    fmt,
  va_list         args);

/* --- public API --- */

/**
 * Global init function - register codecs.
 */
AUDEC_SYMBOL_EXPORT
void
audec_init (void);

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
AUDEC_SYMBOL_EXPORT
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
AUDEC_SYMBOL_EXPORT
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
AUDEC_SYMBOL_EXPORT
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
AUDEC_SYMBOL_EXPORT
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
AUDEC_SYMBOL_EXPORT
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
AUDEC_SYMBOL_EXPORT
void
audec_clear_nfo (
  AudecInfo * nfo);

/** free possibly allocated meta-data text
 * @param nfo pointer to a adinfo struct
 */
AUDEC_SYMBOL_EXPORT
void
audec_free_nfo (
  AudecInfo * nfo);


/* --- helper functions --- */

/**
 * Read file info.
 *
 * Combines ad_open() and ad_close().
 */
AUDEC_SYMBOL_EXPORT
int
audec_finfo (
  const char * filename,
  AudecInfo *  nfo);

/**
 * Wrapper around \ref audec_read, downmixes all channels to
 * mono.
 */
AUDEC_SYMBOL_EXPORT
ssize_t
audec_read_mono_dbl (
  void *, AudecInfo *, double*, size_t, int);

/**
 * Calls dbg() to print file info to stderr.
 *
 * @param dbglvl
 * @param info
 */
AUDEC_SYMBOL_EXPORT
void
audec_dump_info (
  AudecLogLevel dbglvl,
  AudecInfo *     nfo);

/**
 * Specify a log callback for audec to write
 * log entries to.
 *
 * If this is not set, libaudec will write
 * warnings and errors to stderr.
 */
AUDEC_SYMBOL_EXPORT
void
audec_set_log_func (
  audec_log_fn_t log_fn);

/**
 * Set audio-decoder debug level.
 *
 * All info is printed to stderr.
 *
 * @param lvl debug-level threshold.
 */
AUDEC_SYMBOL_EXPORT
void
audec_set_log_level (
  AudecLogLevel lvl);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __AUDEC_AUDEC_H__ */
