plugins_ldflags += ['-Wl,--undefined=all_tasks_panel_plugin_register_types']

sources = files(
  'all-tasks-panel-plugin.c',
  'gtd-all-tasks-panel.c'
)

plugins_libs += static_library(
      'alltaskspanel',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps
)
