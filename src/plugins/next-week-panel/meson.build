plugins_ldflags += ['-Wl,--undefined=next_week_panel_plugin_register_types']

plugins_sources += files(
  'gtd-next-week-panel.c',
  'next-week-panel-plugin.c'
)

plugins_sources += gnome.compile_resources(
  'next-week-panel-resources',
  'next-week-panel.gresource.xml',
  c_name: 'next_week_panel_plugin',
)
