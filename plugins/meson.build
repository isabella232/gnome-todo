plugins_incs = [top_inc, src_inc]

plugins_ldflags = []
plugins_libs = []
plugins_confs = []

plugins_conf = configuration_data()
plugins_conf.set('VERSION', gnome_todo_version)

subdir('all-tasks-panel')
subdir('background')
subdir('dark-theme')
subdir('eds')
subdir('inbox-panel')
subdir('next-week-panel')
subdir('night-light')
subdir('scheduled-panel')
subdir('peace')
subdir('task-lists-workspace')
subdir('today-panel')

if get_option('score_plugin')
  subdir('score')
endif

if get_option('unscheduled_panel_plugin')
  subdir('unscheduled-panel')
endif

if get_option('todo_txt_plugin')
  subdir('todo-txt')
endif

if get_option('todoist_plugin')
  subdir('todoist')
endif

plugins_dep = declare_dependency(
  link_with: plugins_libs,
  link_args: plugins_ldflags,
)
