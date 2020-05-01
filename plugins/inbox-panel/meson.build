plugins_ldflags += ['-Wl,--undefined=inbox_panel_plugin_register_types']

sources = files(
  'gtd-inbox-panel.c',
  'inbox-panel-plugin.c'
)

plugins_libs += static_library(
         'inboxpanel',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps
)
