plugin_sources = [
  'applications-page.cpp',
  'category.cpp',
  'category-button.cpp',
  'command.cpp',
  'command-edit.cpp',
  'element.cpp',
  'favorites-page.cpp',
  'icon-renderer.cpp',
  'icon-size.cpp',
  'image-menu-item.h',
  'launcher.cpp',
  'launcher-icon-view.cpp',
  'launcher-tree-view.cpp',
  'page.cpp',
  'plugin.cpp',
  'profile.cpp',
  'query.cpp',
  'recent-page.cpp',
  'register-plugin.c',
  'resizer.cpp',
  'run-action.cpp',
  'search-action.cpp',
  'search-page.cpp',
  'settings.cpp',
  'settings-dialog.cpp',
  'slot.h',
  'window.cpp',
  xfce_revision_h,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_module(
  'whiskermenu',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  cpp_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-whiskermenu-plugin'),
    '-DSETTINGS_MENUFILE="@0@"'.format(get_option('sysconfdir') / 'xdg' / 'menus' / 'xfce-settings-manager.menu'),
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    accountsservice,
    exo,
    garcon,
    gio,
    glib,
    gtk,
    gtk_layer_shell,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

popup_whiskermenu_sources = [
  'xfce4-popup-whiskermenu.cpp',
  xfce_revision_h,
]

popup_whiskermenu = executable(
  'xfce4-popup-whiskermenu',
  popup_whiskermenu_sources,
  dependencies: [
    gio,
    glib,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)

i18n.merge_file(
  input: 'whiskermenu.desktop.in',
  output: 'whiskermenu.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)

install_man('xfce4-popup-whiskermenu.1')
