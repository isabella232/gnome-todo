plugins_ldflags += ['-Wl,--undefined=planning_workspace_plugin_register_types']

plugins_sources += files(
  'gtd-inbox-planner.c',
  'gtd-planning-workspace.c',
  'gtd-provider-section.c',
  'gtd-task-card.c',
  'gtd-task-list-card.c',
  'planning-workspace-plugin.c',
)

plugins_sources += gnome.compile_resources(
  'planning-workspace',
  'planning-workspace.gresource.xml',
       c_name: 'planning_workspace',
       export: true,
)
