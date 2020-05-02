plugins_ldflags += ['-Wl,--undefined=task_lists_workspace_plugin_register_types']

task_lists_panel_sources = files(
  'gtd-sidebar.c',
  'gtd-sidebar-list-row.c',
  'gtd-sidebar-panel-row.c',
  'gtd-sidebar-provider-row.c',
  'gtd-task-list-panel.c',
  'gtd-task-lists-workspace.c',
  'task-lists-workspace-plugin.c',
)

task_lists_panel_sources += gnome.compile_resources(
  'task-lists-workspace',
  'task-lists-workspace.gresource.xml',
       c_name: 'task_lists_workspace',
       export: true,
)

plugins_sources += task_lists_panel_sources
