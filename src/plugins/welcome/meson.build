plugins_ldflags += ['-Wl,--undefined=welcome_plugin_register_types']

plugins_sources += files(
  'gtd-welcome-action-card.c',
  'gtd-welcome-workspace.c',
  'welcome-plugin.c',
)

plugins_sources += gnome.compile_resources(
  'welcome-resources',
  'welcome.gresource.xml',
  c_name: 'welcome_plugin',
)
