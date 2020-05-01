plugins_ldflags += ['-Wl,--undefined=gtd_plugin_eds_register_types']

################
# Dependencies #
################

plugins_deps += [
  dependency('libecal-2.0', version: '>= 3.33.2'),
  dependency('libedataserver-1.2', version: '>= 3.32.0'),
]

plugins_sources += files(
  'e-source-gnome-todo.c',
  'gtd-plugin-eds.c',
  'gtd-provider-eds.c',
  'gtd-provider-goa.c',
  'gtd-provider-local.c',
  'gtd-task-eds.c',
  'gtd-task-list-eds.c',
)
