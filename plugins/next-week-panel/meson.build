plugins_ldflags += ['-Wl,--undefined=next_week_panel_plugin_register_types']

sources = files(
  'gtd-next-week-panel.c',
  'next-week-panel-plugin.c'
)

plugins_libs += static_library(
      'nextweekpanel',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps
)
