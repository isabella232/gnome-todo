plugins_ldflags += ['-Wl,--undefined=peace_plugin_register_types']

plugins_sources += files(
  'gtd-peace-omni-area-addin.c',
  'peace-plugin.c',
)

plugins_sources += gnome.compile_resources(
  'peace-resources',
  'peace.gresource.xml',
  c_name: 'peace_plugin',
)
