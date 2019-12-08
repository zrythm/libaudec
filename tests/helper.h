/*
 * Copyright (C) 2019 Alexandros Theodotou <alex at zrythm dot org>

 * This file is part of libaudec

 * libaudec is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * libaudec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with libaudec.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __AD_TESTS_HELPER_H__
#define __AD_TESTS_HELPER_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ad_printf(a,...) \
  fprintf (stderr , a, ##__VA_ARGS__)

#define ad_assert(x) \
  if (!(x)) \
    { \
      ad_printf("Assertion failed: %s", #x); \
      exit(1); \
    }

#endif
