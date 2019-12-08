libaudec
========

libaudec (lib audio decoder) is a wrapper library
over ffmpeg, sndfile and libsamplerate for reading
and resampling audio files, based on Robin Gareus'
`audio_decoder` code
(https://github.com/x42/silan/tree/master/audio_decoder)

This library is meant to be linked in statically
to larger projects.

# Usage

Use `#include <audec/audec.h>` in your C/C++
project and add a dependency to the library using
the pkg-config name `audec`.

Example of loading a file into an interleaved
float array at 44000Hz:

    AudecInfo nfo;
    AudecHandle * handle =
      audec_open (filename, &nfo);
    if (!handle)
      {
        /* handle error */
      }
    float * out_frames = NULL;
    ssize_t samples_read =
      audec_read (
        handle, &out_frames, 44000);
    if (samples_read < 0)
      {
        /* handle error */
      }
    audec_close (handle);

See the header file for more info.

# Building

To build only:

    meson build
    ninja -C build

Installation:

    ninja -C build install

# License
libaudec is released under the GNU LGPLv3+. See the
file COPYING for more details. Some files, where
specified, are licensed under different licenses.

----
Copyright (C) 2019 Alexandros Theodotou

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.  This file is offered as-is,
without any warranty.
