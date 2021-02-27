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

void
ad_log (
  const char *    func,
  AudecLogLevel level,
  const char *    format,
  ...);

static void
log_fn (
  AudecLogLevel level,
  const char *    fmt,
  va_list         args)
{
  fprintf (stderr, "using log fn\n");
  vfprintf (stderr, fmt, args);
}

int main (
  int argc, const char* argv[])
{
  ad_assert (argc > 2);

  const char * filename = argv[1];

  audec_init ();

  /* read info */
  AudecInfo nfo;
  memset (&nfo, 0, sizeof (AudecInfo));
  AudecHandle * handle =
    audec_open (filename, &nfo);

  /* test log function */
  audec_set_log_func (log_fn);

  ad_log (
    __func__, AUDEC_LOG_LEVEL_DEBUG,
    "abc %s %s\n", "abc2", "abc3");

  audec_close (handle);

  return 0;
}
