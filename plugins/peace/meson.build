plugins_ldflags += ['-Wl,--undefined=peace_plugin_register_types']

sources = files(
  'gtd-peace-omni-area-addin.c',
  'peace-plugin.c',
)

plugins_libs += static_library(
              'peace',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps,
)
