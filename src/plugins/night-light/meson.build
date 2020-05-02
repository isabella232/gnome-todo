plugins_ldflags += ['-Wl,--undefined=gtd_night_light_plugin_register_types']

plugins_sources += files(
  'gtd-night-light-plugin.c',
)

plugins_sources += gnome.compile_resources(
  'night-light-resources',
  'night-light.gresource.xml',
  c_name: 'night_light_plugin',
)
