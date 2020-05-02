plugins_ldflags += ['-Wl,--undefined=gtd_plugin_background_register_types']

plugins_deps += [
  dependency('libportal'),
]

plugins_sources += files(
  'gtd-plugin-background.c',
)

plugins_sources += gnome.compile_resources(
  'background-resources',
  'background.gresource.xml',
  c_name: 'background_plugin',
)

install_data(
  'org.gnome.todo.background.gschema.xml',
  install_dir: gnome_todo_schemadir
)
