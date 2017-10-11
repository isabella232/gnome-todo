appdata = 'org.gnome.Todo.appdata.xml'

i18n.merge_file(
  appdata,
  input: appdata + '.in',
  output: appdata,
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(gnome_todo_datadir, 'metainfo')
)
