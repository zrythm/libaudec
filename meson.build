# Copyright (C) 2019-2021 Alexandros Theodotou <alex at zrythm dot org>
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

project (
  'libaudec', ['c'],
  version: files ('VERSION'),
  license: 'AGPLv3+',
  meson_version: '>= 0.57.0',
  default_options: [
    'warning_level=2',
    'buildtype=debug',
    'c_std=gnu11',
    ],
  )

cmake = import ('cmake')
cmake_opts = cmake.subproject_options ()
cmake_opts.add_cmake_defines ({'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'})

srcs = []
subdir('inc')
subdir('src')

cc = meson.get_compiler ('c')
is_ld_bfd = cc.get_linker_id () == 'ld.bfd'
prefix = get_option ('prefix')
includedir = join_paths (
  prefix, get_option('includedir'))
libdir = join_paths (prefix, get_option ('libdir'))

cdata = configuration_data ()
cdata.set_quoted (
  'PACKAGE_VERSION',
  '@VCS_TAG@')
cdata.set_quoted (
  'COMPILER',
  meson.get_compiler('c').get_id())
cdata.set_quoted (
  'PREFIX', prefix)
cdata.set_quoted (
  'COMPILER_VERSION',
  meson.get_compiler('c').version())
cdata.set_quoted (
  'CONFIGURE_LIBDIR',
  libdir)

# detect OS
os_darwin = false
os_linux = false
os_freebsd = false
os_windows = false
if host_machine.system() == 'darwin'
  os_darwin = true
elif host_machine.system() == 'linux'
  os_linux = true
elif host_machine.system() == 'freebsd'
  os_freebsd = true
elif host_machine.system() == 'windows'
  os_windows = true
endif

inc = [
  include_directories ('.'),
  include_directories ('inc'),
  ]

# Compiler flags
audec_cflags = [
  '-Wall',
  '-Wformat=2',
  '-Wno-missing-field-initializers',
  '-Wno-unused-parameter',
  '-Wno-sequence-point',
  '-Wignored-qualifiers',
  '-Wno-cast-function-type',
  '-Werror=cast-qual',
  '-Werror=clobbered',
  '-Werror=conversion',
  '-Werror=disabled-optimization',
  '-Werror=double-promotion',
  '-Werror=float-equal',
  '-Werror=logical-op',
  '-Werror=pointer-arith',
  '-Werror=sign-conversion',
  '-Werror=overlength-strings',
  '-Werror=stringop-truncation',
  '-Werror=missing-declarations',
  '-Werror=redundant-decls',
  '-Werror=shadow',
  '-Werror=undef',
  '-Werror=unused',
  '-Werror=strict-aliasing',
  '-fstrict-aliasing',
  '-Werror=strict-overflow',
  '-Wstrict-overflow=2',
  '-fstrict-overflow',
  '-Werror=duplicated-branches',
  '-Werror=duplicated-cond',
  '-Werror=null-dereference',
  '-Werror=init-self',
  '-Werror=jump-misses-init',
  '-Werror=missing-prototypes',
  '-Werror=nested-externs',
  '-Werror=write-strings',
  '-Werror=implicit-fallthrough',
  '-Werror=sign-compare',
  '-Werror=discarded-qualifiers',
  '-Werror=float-conversion',
  '-Werror=implicit-function-declaration',
  '-Werror=uninitialized',
  '-Werror=maybe-uninitialized',
  '-Werror=return-type',
  '-Werror=int-conversion',
  '-Werror=format-security',
  '-Werror=incompatible-pointer-types',
  '-Werror=implicit-int',
  '-Werror=multistatement-macros',
  '-Werror=switch',
  '-Werror=overflow',
  '-Werror=array-bounds',
  '-Werror=enum-compare',
  '-Werror=misleading-indentation',
  '-Werror=int-in-bool-context',
  '-Werror=type-limits',
  '-Werror=deprecated-declarations',
  '-fvisibility=hidden',
  ]
if os_windows
  audec_cflags += [
    '-D_WOE32=1',
    ]
endif
audec_cflags = cc.get_supported_arguments (
  audec_cflags)

samplerate_dep = dependency (
  'samplerate', version: '>=0.1.8', required: false)
if not samplerate_dep.found ()
  samplerate_subproject = cmake.subproject (
    'samplerate', options: cmake_opts)
  samplerate_dep = samplerate_subproject.dependency (
    'samplerate')
endif
sndfile_dep = dependency (
  'sndfile', version: '>=1.0.25', required: false)
if not sndfile_dep.found ()
  sndfile_subproject = cmake.subproject (
    'sndfile', options: cmake_opts)
  sndfile_dep = sndfile_subproject.dependency (
    'sndfile')
endif
# this is not enough - need to check dependencies for
# libmp3lame because most distros build without it
if sndfile_dep.version ().version_compare ('>=1.1.0') and false
  cdata.set (
    'LIBSNDFILE_HAVE_MP3', 0)
endif

# Maths functions might be implemented in libm
libm = cc.find_library (
  'm', required: false)

audec_deps = [
  sndfile_dep,
  samplerate_dep,
  libm,
  ]

# create config.h and add to deps
tmp_h = configure_file (
  output: 'tmp.h',
  configuration: cdata,
  )
config_h_vcs = vcs_tag (
  input: tmp_h,
  output: 'config.h',
  )
config_h_dep = declare_dependency (
  sources: config_h_vcs,
  )
audec_deps += config_h_dep

mapfile = configure_file(
  input: 'link.map.in',
  output: 'link.map',
  configuration: { 'VERSION': meson.project_version () },
)
vflag = '-Wl,--version-script,@0@'.format (mapfile)

audec = both_libraries (
  'audec',
  sources: srcs,
  dependencies: [
    audec_deps,
    ],
  include_directories: inc,
  link_args: is_ld_bfd ? vflag : [],
  link_depends: is_ld_bfd ? mapfile : [],
  install: not meson.is_subproject(),
)

# this is so that it can be used as a meson
# subproject
libaudec_dep = declare_dependency (
  include_directories: inc,
  dependencies: audec_deps,
  link_with: audec.get_static_lib ())

if not meson.is_subproject()
  pkg_mod = import('pkgconfig')
  pkg_mod.generate(
    libraries: audec,
    version: meson.project_version (),
    name: 'audec',
    filebase: 'audec',
    description: 'A library to read and resample audio files.',
    )
endif

audec_exe = executable (
  'audec',
  'src/main.c',
  dependencies: libaudec_dep,
  install: not meson.is_subproject(),
  )

subdir('tests')

summary = [
  '',
  '------',
  'libaudec @0@'.format(meson.project_version()),
  '',
  '  Build type: @0@'.format(
    get_option('buildtype')),
  '  Directories:',
  '            prefix: @0@'.format(prefix),
  '        includedir: @0@'.format(includedir),
  '            libdir: @0@'.format(libdir),
  '------',
  ''
]
message('\n'.join(summary))
