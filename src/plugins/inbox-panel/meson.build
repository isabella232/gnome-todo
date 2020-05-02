plugins_ldflags += ['-Wl,--undefined=inbox_panel_plugin_register_types']

plugins_sources += files(
  'gtd-inbox-panel.c',
  'inbox-panel-plugin.c'
)

plugins_sources += gnome.compile_resources(
  'inbox-panel-resources',
  'inbox-panel.gresource.xml',
  c_name: 'inbox_panel_plugin',
)
