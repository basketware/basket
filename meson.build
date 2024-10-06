project(
  'basket',
  'c',
  version: '0.1',
  default_options: ['warning_level=3', 'c_std=c11'],
)

cc = meson.get_compiler('c')

sdl2_dep = dependency('sdl2', static: true)

if host_machine.system() == 'linux'
  cc = meson.get_compiler('c')
  m_dep = cc.find_library('m', required: true)
else
  m_dep = declare_dependency()
endif

src_files = files(
  'lib/common.c',
  'lib/mojoal.c',
  'lib/tinyfx.c',
  'lib/vec.c',
  'lib/vec.h',
  'lib/zip.c',
  'audio.c',
  'engine.c',
  'error.c',
  'filesystem.c',
  'image.c',
  'input.c',
  'mafs.c',
  'model.c',
  'pool.c',
  'renderer.c',
)

inc_dirs = [include_directories('lib'), include_directories('.')]

basket_lib = static_library(
  'basket',
  src_files,
  include_directories: inc_dirs,
  dependencies: [sdl2_dep, m_dep],
  install: true,
)

basket_dep = declare_dependency(
  link_with: basket_lib,
  include_directories: '.',
  dependencies: [sdl2_dep, m_dep],
)

install_headers('basket.h')

# Vala bindings :)

glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')

basket_vala_lib = static_library(
  'basket-vala',
  'vala/vala.c',
  include_directories: ['.', 'vala'],
  dependencies: [basket_dep, glib_dep, gobject_dep],
  install: true,
)

basket_vala_dep = declare_dependency(
  link_with: basket_vala_lib,
  include_directories: ['.', 'vala'],
  dependencies: [basket_dep, glib_dep, gobject_dep],
)
