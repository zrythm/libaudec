# Copyright (C) 2019-2020 Alexandros Theodotou <alex at zrythm dot org>
#
# This file is part of libaudec
#
# libaudec is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libaudec is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with libaudec.  If not, see <https://www.gnu.org/licenses/>.

if get_option ('tests')
  e = executable (
    'test_exe', 'test.c',
    include_directories: inc,
    link_with: audec,
    c_args: audec_cflags,
    )
  test (
    'wav_test', e,
    args: [
      join_paths (
        meson.current_source_dir(), 'test.wav'),
      '44000',
      ])
  test (
    'mp3_test', e,
    args: [
      join_paths (
        meson.current_source_dir(), 'test.mp3'),
      '48000',
      ])
  log_test = executable (
    'log_test_exe', 'log.c',
    include_directories: inc,
    link_with: audec.get_static_lib (),
    c_args: audec_cflags,
    )
  test (
    'log_test', log_test,
    args: [
      join_paths (
        meson.current_source_dir(), 'test.wav'),
      '44000',
      ])
endif
