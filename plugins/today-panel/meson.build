plugin_name = 'today-panel'

plugins_ldflags += ['-Wl,--undefined=today_panel_plugin_register_types']

sources = files(
  'gtd-panel-today.c',
  'gtd-today-omni-area-addin.c',
  'today-panel-plugin.c'
)

plugins_libs += static_library(
         'todaypanel',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps,
                  pic: true,
)
