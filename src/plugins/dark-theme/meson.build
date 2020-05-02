plugins_ldflags += ['-Wl,--undefined=gtd_plugin_dark_theme_register_types']

plugins_sources += files(
  'gtd-plugin-dark-theme.c'
)

plugins_sources += gnome.compile_resources(
  'dark-theme-resources',
  'dark-theme.gresource.xml',
  c_name: 'dark_theme_plugin',
)
