plugins_ldflags += ['-Wl,--undefined=all_tasks_panel_plugin_register_types']

plugins_sources += files(
  'all-tasks-panel-plugin.c',
  'gtd-all-tasks-panel.c'
)

plugins_sources += gnome.compile_resources(
  'all-tasks-panel-resources',
  'all-tasks-panel.gresource.xml',
  c_name: 'all_tasks_panel_plugin',
)
