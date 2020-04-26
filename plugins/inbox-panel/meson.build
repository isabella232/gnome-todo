plugins_ldflags += ['-Wl,--undefined=gtd_plugin_inbox_panel_register_types']

sources = files(
  'gtd-inbox-panel.c',
  'gtd-plugin-inbox-panel.c'
)

plugins_libs += static_library(
         'inboxpanel',
              sources: sources,
  include_directories: plugins_incs,
         dependencies: gnome_todo_deps
)

plugins_confs += configure_file(
          input: 'inbox-panel.plugin.in',
         output: 'inbox-panel.plugin',
  configuration: plugins_conf
)
