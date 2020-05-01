plugins_ldflags += ['-Wl,--undefined=gtd_plugin_background_register_types']

plugins_deps += [
  dependency('libportal'),
]

plugins_sources += files(
  'gtd-plugin-background.c',
)

install_data(
  'org.gnome.todo.background.gschema.xml',
  install_dir: gnome_todo_schemadir
)
