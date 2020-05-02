plugins_ldflags += ['-Wl,--undefined=today_panel_plugin_register_types']

plugins_sources += files(
  'gtd-panel-today.c',
  'gtd-today-omni-area-addin.c',
  'today-panel-plugin.c'
)


plugins_sources += gnome.compile_resources(
  'today-panel-resources',
  'today-panel.gresource.xml',
  c_name: 'today_panel_plugin',
)
