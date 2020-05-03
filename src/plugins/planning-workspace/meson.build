plugins_ldflags += ['-Wl,--undefined=planning_workspace_plugin_register_types']

plugins_sources += files(
  'gtd-planning-workspace.c',
  'planning-workspace-plugin.c',
)

plugins_sources += gnome.compile_resources(
  'planning-workspace',
  'planning-workspace.gresource.xml',
       c_name: 'planning_workspace',
       export: true,
)
