/*
 * Copyright (C) 2020 Alexandros Theodotou <alex at zrythm dot org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ad_plugin.h"
#include "audec/audec.h"

int
main (int    argc,
      char **argv)
{
  if (argc < 2)
    {
      printf ("Need at least 1 argument\n");
      exit (-1);
    }

  char * command = argv[1];
  char * arg2 = NULL;
  char * arg3 = NULL;
  if (argc > 2)
    arg2 = argv[2];
  if (argc > 3)
    arg3 = argv[3];

  audec_init ();

  if (!strcmp (command, "info"))
    {
      if (!arg2)
        {
          printf ("No file provided\n");
          exit (-1);
        }

      AudecInfo nfo;
      AudecHandle * handle =
        audec_open (arg2, &nfo);
      audec_dump_info (AUDEC_DEBUG_LEVEL_ERROR, &nfo);
      int ret = audec_close (handle);
      if (ret)
        {
          printf ("An error occured closing handle\n");
          exit (-1);
        }
    }

  if (!strcmp (command, "read"))
    {
      if (!arg2)
        {
          printf ("No file provided\n");
          exit (-1);
        }
      if (!arg3)
        {
          printf ("No samplerate provided\n");
          exit (-1);
        }

      AudecInfo nfo;
      AudecHandle * handle =
        audec_open (arg2, &nfo);
      float * out_frames = NULL;
      ssize_t samples_read =
        audec_read (
          handle, &out_frames, atoi (arg3));
      printf ("Samples read: %zu\n", samples_read);
      int ret = audec_close (handle);
      if (ret)
        {
          printf ("An error occured closing handle\n");
          exit (-1);
        }
    }

  return 0;
}
