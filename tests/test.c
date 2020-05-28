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
 */

#include "helper.h"

#include <audec/audec.h>

int main (
  int argc, const char* argv[])
{
  ad_assert (argc > 2);

  const char * filename = argv[1];
  int sample_rate = atoi (argv[2]);

  size_t expected_frames_before = 0;
  unsigned int expected_sample_rate_before = 0;
  unsigned int expected_channels_before = 0;
  if (str_endswith (filename, "test.wav"))
    {
      expected_frames_before = 164571;
      expected_sample_rate_before = 48000;
      expected_channels_before = 2;
    }
  else if (str_endswith (filename, "test.mp3"))
    {
      /*expected_frames_before = 294912;*/
      /* FIXME audacity says above */
      expected_frames_before = 292654;
      expected_sample_rate_before = 44100;
      expected_channels_before = 2;
    }
  size_t expected_frames_after =
    (size_t)
    ((double) expected_frames_before *
    ((double) sample_rate /
     expected_sample_rate_before));

  audec_init ();

  /* read info */
  AudecInfo nfo;
  AudecHandle * handle =
    audec_open (filename, &nfo);
  ad_assert (
    expected_sample_rate_before == nfo.sample_rate);
  ad_assert (
    expected_frames_before == (size_t) nfo.frames);
  ad_assert (
    expected_channels_before == nfo.channels);

  /* read frames */
  float * out_frames = NULL;
  ssize_t samples_read =
    audec_read (
      handle, &out_frames, sample_rate);
  ad_printf ("samples read %zd", samples_read);
  ad_assert (
    samples_read > 0);
  ad_assert (
    expected_frames_after == (size_t) samples_read);

  /* try to access the last element (valgrind will
   * detect this if array is not large enough) */
  float frame =
    out_frames[samples_read * nfo.channels - 1];
  (void) frame;

  audec_close (handle);
  free (out_frames);

  return 0;
}
