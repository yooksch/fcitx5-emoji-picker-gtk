project('fcitx5-emoji-picker-gtk', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++23'])

shared_library(
  'fcitx5-emoji-picker-gtk',
  [
    'src/fcitx5-module.cpp',
    'src/window.cpp',
    'src/emojis.cpp',
    'src/emoji-button.cpp'
  ],
  include_directories: 'include',
  install : true,
  install_dir : '/usr/lib/fcitx5/',
  dependencies: [
    dependency('gtkmm-3.0', required: true),
    dependency('gtk-layer-shell-0', required : true),
    dependency('Fcitx5Module', required : true, modules : ['Core'])
  ]
)

install_data(
  ['fcitx5-emoji-picker-gtk.conf'],
  install_dir : 'share/fcitx5/addon'
)
