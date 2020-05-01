plugin_name = 'task-lists-workspace'

plugins_ldflags += ['-Wl,--undefined=task_lists_workspace_plugin_register_types']

plugin_data = plugin_name + '.plugin'

sources = files(
  'sidebar/gtd-sidebar.c',
  'sidebar/gtd-sidebar-list-row.c',
  'sidebar/gtd-sidebar-panel-row.c',
  'sidebar/gtd-sidebar-provider-row.c',
  'gtd-task-list-panel.c',
  'gtd-task-lists-workspace.c',
  'task-lists-workspace-plugin.c',
)

sources += gnome.compile_resources(
  'task-lists-workspace',
  'task-lists-workspace.gresource.xml',
       c_name: 'task_lists_workspace',
       export: true,
)

plugins_libs += static_library(
          plugin_name,
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps
)
