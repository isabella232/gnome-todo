/* gtd-task-list-view.c
 *
 * Copyright (C) 2015-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "GtdTaskListView"

#include "gtd-debug.h"
#include "gtd-dnd-row.h"
#include "gtd-edit-pane.h"
#include "gtd-empty-list-widget.h"
#include "gtd-task-list-view.h"
#include "gtd-manager.h"
#include "gtd-markdown-renderer.h"
#include "gtd-new-task-row.h"
#include "gtd-notification.h"
#include "gtd-provider.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-task-list-view-model.h"
#include "gtd-task-row.h"
#include "gtd-utils-private.h"
#include "gtd-widget.h"
#include "gtd-window.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

/**
 * SECTION:gtd-task-list-view
 * @Short_description: A widget to display tasklists
 * @Title:GtdTaskListView
 *
 * The #GtdTaskListView widget shows the tasks of a #GtdTaskList with
 * various options to fine-tune the appearance. Alternatively, one can
 * pass a #GList of #GtdTask objects.
 *
 * It supports custom sorting and header functions, so the tasks can be
 * sorted in various ways. See the "Today" and "Scheduled" panels for reference
 * implementations.
 *
 * Example:
 * |[
 * GtdTaskListView *view = gtd_task_list_view_new ();
 *
 * gtd_task_list_view_set_model (view, model);
 *
 * // Date which tasks will be automatically assigned
 * gtd_task_list_view_set_default_date (view, now);
 * ]|
 *
 */

typedef struct
{
  GtkListView           *listview;
  GtkWidget             *scrolled_window;
  GtkStack              *stack;

  /* internal */
  gboolean               can_toggle;
  gboolean               show_due_date;
  gboolean               show_list_name;
  gboolean               handle_subtasks;
  GDateTime             *default_date;

  GtdTaskListViewModel  *model;
  gpointer               active_item;
  gint64                 active_position;

  guint                  scroll_to_bottom_handler_id;

  /* Markup renderer*/
  GtdMarkdownRenderer   *renderer;

  /* DnD */
  GtkListBoxRow         *highlighted_row;
  guint                  scroll_timeout_id;
  gboolean               scroll_up;

  /* action */
  GActionGroup          *action_group;

  /* Custom header function data */
  GtdTaskListViewHeaderFunc header_func;
  gpointer                  header_user_data;

  GtkSizeGroup           *due_date_sizegroup;
  GtkSizeGroup           *tasklist_name_sizegroup;
} GtdTaskListViewPrivate;

struct _GtdTaskListView
{
  GtkBox                  parent;

  /*<private>*/
  GtdTaskListViewPrivate *priv;
};

#define COLOR_TEMPLATE               "tasklistview {background-color: %s;}"
#define DND_SCROLL_OFFSET            24 //px
#define LUMINANCE(c)                 (0.299 * c->red + 0.587 * c->green + 0.114 * c->blue)
#define TASK_REMOVED_NOTIFICATION_ID "task-removed-id"


static void          on_clear_completed_tasks_activated_cb       (GSimpleAction      *simple,
                                                                  GVariant           *parameter,
                                                                  gpointer            user_data);

static void          on_remove_task_row_cb                       (GtdTaskRow         *row,
                                                                  GtdTaskListView    *self);

static gboolean      scroll_to_bottom_cb                         (gpointer            data);


G_DEFINE_TYPE_WITH_PRIVATE (GtdTaskListView, gtd_task_list_view, GTK_TYPE_BOX)

static const GActionEntry gtd_task_list_view_entries[] = {
  { "clear-completed-tasks", on_clear_completed_tasks_activated_cb },
};

typedef struct
{
  GtdTaskListView *view;
  GtdTask         *task;
} RemoveTaskData;

enum {
  PROP_0,
  PROP_HANDLE_SUBTASKS,
  PROP_SHOW_LIST_NAME,
  PROP_SHOW_DUE_DATE,
  LAST_PROP
};

typedef gboolean     (*IterateSubtaskFunc)                       (GtdTaskListView    *self,
                                                                  GtdTask            *task);

/*
 * Auxiliary methods
 */

static gboolean
iterate_subtasks (GtdTaskListView    *self,
                  GtdTask            *task,
                  IterateSubtaskFunc  func)
{
  GtdTask *aux;

  if (!func (self, task))
    return FALSE;

  for (aux = gtd_task_get_first_subtask (task);
       aux;
       aux = gtd_task_get_next_sibling (aux))
    {
      if (!iterate_subtasks (self, aux, func))
        return FALSE;
    }

  return TRUE;
}

static void
schedule_scroll_to_bottom (GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv = gtd_task_list_view_get_instance_private (self);

  if (priv->scroll_to_bottom_handler_id > 0)
    return;

  priv->scroll_to_bottom_handler_id = g_timeout_add (250, scroll_to_bottom_cb, self);
}

static GtkWidget*
create_task_row (GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv = gtd_task_list_view_get_instance_private (self);
  GtkWidget *row;

  row = gtd_task_row_new (NULL, priv->renderer);

  g_object_bind_property (self,
                          "handle-subtasks",
                          row,
                          "handle-subtasks",
                          G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

  gtd_task_row_set_list_name_visible (GTD_TASK_ROW (row), priv->show_list_name);
  gtd_task_row_set_due_date_visible (GTD_TASK_ROW (row), priv->show_due_date);
  gtd_task_row_set_sizegroups (GTD_TASK_ROW (row),
                               priv->tasklist_name_sizegroup,
                               priv->due_date_sizegroup);

  g_signal_connect (row, "remove-task", G_CALLBACK (on_remove_task_row_cb), self);

  return row;
}


/*
 * Callbacks
 */

static gboolean
scroll_to_bottom_cb (gpointer data)
{
  GtdTaskListViewPrivate *priv;
  GtkWidget *widget;
  GtkRoot *root;

  priv = gtd_task_list_view_get_instance_private (data);
  widget = GTK_WIDGET (data);
  root = gtk_widget_get_root (widget);

  if (!root)
    return G_SOURCE_CONTINUE;

  priv->scroll_to_bottom_handler_id = 0;

  /*
   * Only focus the new task row if the current list is visible,
   * and the focused widget isn't inside this list view.
   */
  if (gtk_widget_get_visible (widget) &&
      gtk_widget_get_child_visible (widget) &&
      gtk_widget_get_mapped (widget) &&
      !gtk_widget_is_ancestor (gtk_window_get_focus (GTK_WINDOW (root)), widget))
    {
      gboolean ignored;

      g_signal_emit_by_name (priv->scrolled_window, "scroll-child", GTK_SCROLL_END, FALSE, &ignored);
    }

  return G_SOURCE_REMOVE;
}

static void
on_task_removed_cb (GObject      *source,
                    GAsyncResult *result,
                    gpointer      user_data)
{
  g_autoptr (GError) error = NULL;

  gtd_provider_remove_task_finish (GTD_PROVIDER (source), result, &error);

  if (error)
    g_warning ("Error removing task list: %s", error->message);
}

static inline gboolean
remove_task_cb (GtdTaskListView *self,
                GtdTask         *task)
{
  gtd_provider_remove_task (gtd_task_get_provider (task),
                            task,
                            NULL,
                            on_task_removed_cb,
                            self);
  return TRUE;
}

static void
on_clear_completed_tasks_activated_cb (GSimpleAction *simple,
                                       GVariant      *parameter,
                                       gpointer       user_data)
{
  GtdTaskListView *self;
  GListModel *model;
  guint i;

  self = GTD_TASK_LIST_VIEW (user_data);
  model = gtd_task_list_view_model_get_model (self->priv->model);

  for (i = 0; i < g_list_model_get_n_items (model); i++)
    {
      g_autoptr (GtdTask) task = g_list_model_get_item (model, i);

      if (!gtd_task_get_complete (task))
        continue;

      if (gtd_task_get_parent (task))
        gtd_task_remove_subtask (gtd_task_get_parent (task), task);

      /* Remove the subtasks recursively */
      iterate_subtasks (self, task, remove_task_cb);
    }
}

static void
on_remove_task_action_cb (GtdNotification *notification,
                          gpointer         user_data)
{
  RemoveTaskData *data;
  GtdTask *task;

  data = user_data;
  task = data->task;

  if (gtd_task_get_parent (task))
    gtd_task_remove_subtask (gtd_task_get_parent (task), task);

  /* Remove the subtasks recursively */
  iterate_subtasks (data->view, data->task, remove_task_cb);

  g_clear_pointer (&data, g_free);
}

static void
on_undo_remove_task_action_cb (GtdNotification *notification,
                               gpointer         user_data)
{
  RemoveTaskData *data;
  GtdTaskList *list;

  data = user_data;

  /*
   * Readd task to the list. This will emit GListModel:items-changed (since
   * GtdTaskList implements GListModel) and the row will be added back.
   */
  list = gtd_task_get_list (data->task);
  gtd_task_list_add_task (list, data->task);

  g_free (data);
}

static void
on_remove_task_row_cb (GtdTaskRow      *row,
                       GtdTaskListView *self)
{
  g_autofree gchar *text = NULL;
  GtdNotification *notification;
  RemoveTaskData *data;
  GtdTaskList *list;
  GtdWindow *window;
  GtdTask *task;

  task = gtd_task_row_get_task (row);

  text = g_strdup_printf (_("Task <b>%s</b> removed"), gtd_task_get_title (task));
  window = GTD_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));

  data = g_new0 (RemoveTaskData, 1);
  data->view = self;
  data->task = task;

  /* Remove tasks and subtasks from the list */
  list = gtd_task_get_list (task);
  gtd_task_list_remove_task (list, task);

  /* Notify about the removal */
  notification = gtd_notification_new (text, 5000.0);

  gtd_notification_set_primary_action (notification,
                                       (GtdNotificationActionFunc) on_remove_task_action_cb,
                                       data);

  gtd_notification_set_secondary_action (notification,
                                         _("Undo"),
                                         (GtdNotificationActionFunc) on_undo_remove_task_action_cb,
                                         data);

  gtd_window_notify (window, notification);


  /* Clear the active row */
  gtd_task_row_set_active (row, FALSE);
}

static void
on_listview_activate_cb (GtkListBox      *listbox,
                         guint            position,
                         GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv = gtd_task_list_view_get_instance_private (self);
  gpointer item;
  gint64 old_active_position;

  GTD_ENTRY;

  if (priv->active_position == position)
    GTD_RETURN ();

  item = g_list_model_get_item (G_LIST_MODEL (priv->model), position);

  old_active_position = priv->active_position;

  priv->active_item = item;
  priv->active_position = position;

  GTD_TRACE_MSG ("Activating %s at %u", G_OBJECT_TYPE_NAME (item), position);

  if (old_active_position != -1)
    g_list_model_items_changed (G_LIST_MODEL (priv->model), old_active_position, 1, 1);

  g_clear_object (&item);

  GTD_EXIT;
}


/*
 * Custom sorting functions
 */

static void
on_listview_setup_cb (GtkSignalListItemFactory *factory,
                      GtkListItem              *list_item,
                      GtdTaskListView          *self)
{
  GtkWidget *row;

  /* Use a task row here, even if it's the sentinel */
  row = create_task_row (self);

  gtk_list_item_set_child (list_item, row);
}

static void
on_listview_bind_cb (GtkSignalListItemFactory *factory,
                     GtkListItem              *list_item,
                     GtdTaskListView          *self)
{
  GtdTaskListViewPrivate *priv = gtd_task_list_view_get_instance_private (self);
  GtkWidget *row;
  gpointer item;

  item = gtk_list_item_get_item (list_item);
  row = gtk_list_item_get_child (list_item);

  if (GTD_IS_TASK (item))
    {
      GtdTaskRow *task_row;

      if (!GTD_IS_TASK_ROW (row))
        {
          row = create_task_row (self);
          gtk_list_item_set_child (list_item, GTK_WIDGET (row));
        }

      task_row = GTD_TASK_ROW (row);

      gtd_task_row_set_task (task_row, GTD_TASK (item));

      if (gtk_list_item_get_item (list_item) == priv->active_item)
        {
          gboolean active;

          /* Toggle the active state if this is the active item */
          active = gtd_task_row_get_active (task_row);
          g_message ("Row active: %d", active);
          gtd_task_row_set_active (task_row, !active);
        }
      else
        {
          gtd_task_row_set_active (task_row, FALSE);
        }

    }
  else if (GTD_IS_SENTINEL (item))
    {
      GListModel *model = gtd_task_list_view_model_get_model (priv->model);

      if (!GTD_IS_NEW_TASK_ROW (row))
        {
          row = gtd_new_task_row_new ();
          gtk_list_item_set_child (list_item, GTK_WIDGET (row));
        }

      gtd_new_task_row_set_show_list_selector (GTD_NEW_TASK_ROW (row),
                                               !GTD_IS_TASK_LIST (model));
    }
  else
    {
      g_assert_not_reached ();
    }
}


/*
 * GObject overrides
 */

static void
gtd_task_list_view_finalize (GObject *object)
{
  GtdTaskListViewPrivate *priv = GTD_TASK_LIST_VIEW (object)->priv;

  g_clear_handle_id (&priv->scroll_to_bottom_handler_id, g_source_remove);
  g_clear_pointer (&priv->default_date, g_date_time_unref);
  g_clear_object (&priv->renderer);
  g_clear_object (&priv->model);

  G_OBJECT_CLASS (gtd_task_list_view_parent_class)->finalize (object);
}

static void
gtd_task_list_view_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GtdTaskListView *self = GTD_TASK_LIST_VIEW (object);

  switch (prop_id)
    {
    case PROP_HANDLE_SUBTASKS:
      g_value_set_boolean (value, self->priv->handle_subtasks);
      break;

    case PROP_SHOW_DUE_DATE:
      g_value_set_boolean (value, self->priv->show_due_date);
      break;

    case PROP_SHOW_LIST_NAME:
      g_value_set_boolean (value, self->priv->show_list_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_list_view_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GtdTaskListView *self = GTD_TASK_LIST_VIEW (object);

  switch (prop_id)
    {
    case PROP_HANDLE_SUBTASKS:
      gtd_task_list_view_set_handle_subtasks (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_DUE_DATE:
      gtd_task_list_view_set_show_due_date (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_LIST_NAME:
      gtd_task_list_view_set_show_list_name (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_list_view_constructed (GObject *object)
{
  GtdTaskListView *self = GTD_TASK_LIST_VIEW (object);

  G_OBJECT_CLASS (gtd_task_list_view_parent_class)->constructed (object);

  /* action_group */
  self->priv->action_group = G_ACTION_GROUP (g_simple_action_group_new ());

  g_action_map_add_action_entries (G_ACTION_MAP (self->priv->action_group),
                                   gtd_task_list_view_entries,
                                   G_N_ELEMENTS (gtd_task_list_view_entries),
                                   object);
}


/*
 * GtkWidget overrides
 */

static void
gtd_task_list_view_map (GtkWidget *widget)
{
  GtdTaskListViewPrivate *priv;
  GtkRoot *root;

  GTK_WIDGET_CLASS (gtd_task_list_view_parent_class)->map (widget);

  priv = GTD_TASK_LIST_VIEW (widget)->priv;
  root = gtk_widget_get_root (widget);

  /* Clear previously added "list" actions */
  gtk_widget_insert_action_group (GTK_WIDGET (root), "list", NULL);

  /* Add this instance's action group */
  gtk_widget_insert_action_group (GTK_WIDGET (root), "list", priv->action_group);
}

static void
gtd_task_list_view_class_init (GtdTaskListViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_task_list_view_finalize;
  object_class->constructed = gtd_task_list_view_constructed;
  object_class->get_property = gtd_task_list_view_get_property;
  object_class->set_property = gtd_task_list_view_set_property;

  widget_class->map = gtd_task_list_view_map;

  g_type_ensure (GTD_TYPE_EDIT_PANE);
  g_type_ensure (GTD_TYPE_NEW_TASK_ROW);
  g_type_ensure (GTD_TYPE_TASK_ROW);
  g_type_ensure (GTD_TYPE_DND_ROW);
  g_type_ensure (GTD_TYPE_EMPTY_LIST_WIDGET);

  /**
   * GtdTaskListView::handle-subtasks:
   *
   * Whether the list is able to handle subtasks.
   */
  g_object_class_install_property (
        object_class,
        PROP_HANDLE_SUBTASKS,
        g_param_spec_boolean ("handle-subtasks",
                              "Whether it handles subtasks",
                              "Whether the list handles subtasks, or not",
                              TRUE,
                              G_PARAM_READWRITE));

  /**
   * GtdTaskListView::show-list-name:
   *
   * Whether the task rows should show the list name.
   */
  g_object_class_install_property (
        object_class,
        PROP_SHOW_LIST_NAME,
        g_param_spec_boolean ("show-list-name",
                              "Whether task rows show the list name",
                              "Whether task rows show the list name at the end of the row",
                              FALSE,
                              G_PARAM_READWRITE));

  /**
   * GtdTaskListView::show-due-date:
   *
   * Whether due dates of the tasks are shown.
   */
  g_object_class_install_property (
        object_class,
        PROP_SHOW_DUE_DATE,
        g_param_spec_boolean ("show-due-date",
                              "Whether due dates are shown",
                              "Whether due dates of the tasks are visible or not",
                              TRUE,
                              G_PARAM_READWRITE));

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/gtd-task-list-view.ui");

  gtk_widget_class_bind_template_child_private (widget_class, GtdTaskListView, due_date_sizegroup);
  gtk_widget_class_bind_template_child_private (widget_class, GtdTaskListView, listview);
  gtk_widget_class_bind_template_child_private (widget_class, GtdTaskListView, tasklist_name_sizegroup);
  gtk_widget_class_bind_template_child_private (widget_class, GtdTaskListView, scrolled_window);
  gtk_widget_class_bind_template_child_private (widget_class, GtdTaskListView, stack);

  gtk_widget_class_bind_template_callback (widget_class, on_listview_activate_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_listview_setup_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_listview_bind_cb);

  gtk_widget_class_set_css_name (widget_class, "tasklistview");
}

static void
gtd_task_list_view_init (GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv;
  GtkNoSelection *no_selection;

  priv = gtd_task_list_view_get_instance_private (self);

  self->priv = priv;

  priv->active_position = -1;
  priv->can_toggle = TRUE;
  priv->handle_subtasks = TRUE;
  priv->show_due_date = TRUE;
  priv->show_due_date = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->renderer = gtd_markdown_renderer_new ();

  priv->model = gtd_task_list_view_model_new ();
  no_selection = gtk_no_selection_new (G_LIST_MODEL (priv->model));
  gtk_list_view_set_model (priv->listview, GTK_SELECTION_MODEL (no_selection));
}

/**
 * gtd_task_list_view_new:
 *
 * Creates a new #GtdTaskListView
 *
 * Returns: (transfer full): a newly allocated #GtdTaskListView
 */
GtkWidget*
gtd_task_list_view_new (void)
{
  return g_object_new (GTD_TYPE_TASK_LIST_VIEW, NULL);
}

/**
 * gtd_task_list_view_get_model:
 * @view: a #GtdTaskListView
 *
 * Retrieves the #GtdTaskList from @view, or %NULL if none was set.
 *
 * Returns: (transfer none): the #GListModel of @view, or %NULL is
 * none was set.
 */
GListModel*
gtd_task_list_view_get_model (GtdTaskListView *view)
{
  GtdTaskListViewPrivate *priv;

  g_return_val_if_fail (GTD_IS_TASK_LIST_VIEW (view), NULL);

  priv = gtd_task_list_view_get_instance_private (view);
  return gtd_task_list_view_model_get_model (priv->model);
}

/**
 * gtd_task_list_view_set_model:
 * @view: a #GtdTaskListView
 * @model: a #GListModel
 *
 * Sets the internal #GListModel of @view. The model must have
 * its element GType as @GtdTask.
 */
void
gtd_task_list_view_set_model (GtdTaskListView *view,
                              GListModel      *model)
{
  GtdTaskListViewPrivate *priv;

  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (view));
  g_return_if_fail (G_IS_LIST_MODEL (model));

  priv = gtd_task_list_view_get_instance_private (view);

  priv->active_item = NULL;
  priv->active_position = -1;

  gtd_task_list_view_model_set_model (priv->model, model);
  schedule_scroll_to_bottom (view);
}

/**
 * gtd_task_list_view_get_show_list_name:
 * @view: a #GtdTaskListView
 *
 * Whether @view shows the tasks' list names.
 *
 * Returns: %TRUE if @view show the tasks' list names, %FALSE otherwise
 */
gboolean
gtd_task_list_view_get_show_list_name (GtdTaskListView *view)
{
  g_return_val_if_fail (GTD_IS_TASK_LIST_VIEW (view), FALSE);

  return view->priv->show_list_name;
}

/**
 * gtd_task_list_view_set_show_list_name:
 * @view: a #GtdTaskListView
 * @show_list_name: %TRUE to show list names, %FALSE to hide it
 *
 * Whether @view should should it's tasks' list name.
 */
void
gtd_task_list_view_set_show_list_name (GtdTaskListView *view,
                                       gboolean         show_list_name)
{
  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (view));

  if (view->priv->show_list_name != show_list_name)
    {
      view->priv->show_list_name = show_list_name;
      g_object_notify (G_OBJECT (view), "show-list-name");
    }
}

/**
 * gtd_task_list_view_get_show_due_date:
 * @self: a #GtdTaskListView
 *
 * Retrieves whether the @self is showing the due dates of the tasks
 * or not.
 *
 * Returns: %TRUE if due dates are visible, %FALSE otherwise.
 */
gboolean
gtd_task_list_view_get_show_due_date (GtdTaskListView *self)
{
  g_return_val_if_fail (GTD_IS_TASK_LIST_VIEW (self), FALSE);

  return self->priv->show_due_date;
}

/**
 * gtd_task_list_view_set_show_due_date:
 * @self: a #GtdTaskListView
 * @show_due_date: %TRUE to show due dates, %FALSE otherwise
 *
 * Sets whether @self shows the due dates of the tasks or not.
 */
void
gtd_task_list_view_set_show_due_date (GtdTaskListView *self,
                                      gboolean         show_due_date)
{
  GtdTaskListViewPrivate *priv;

  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (self));

  priv = gtd_task_list_view_get_instance_private (self);

  if (priv->show_due_date == show_due_date)
    return;

  priv->show_due_date = show_due_date;
  g_object_notify (G_OBJECT (self), "show-due-date");
}

/**
 * gtd_task_list_view_set_header_func:
 * @view: a #GtdTaskListView
 * @func: (closure user_data) (scope call) (nullable): the header function
 * @user_data: data passed to @func
 *
 * Sets @func as the header function of @view. You can safely call
 * %gtk_list_box_row_set_header from within @func.
 *
 * Do not unref nor free any of the passed data.
 */
void
gtd_task_list_view_set_header_func (GtdTaskListView           *view,
                                    GtdTaskListViewHeaderFunc  func,
                                    gpointer                   user_data)
{
  GtdTaskListViewPrivate *priv;

  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (view));

  priv = view->priv;

  if (func)
    {
      priv->header_func = func;
      priv->header_user_data = user_data;
    }
  else
    {
      priv->header_func = NULL;
      priv->header_user_data = NULL;
    }
}

/**
 * gtd_task_list_view_get_default_date:
 * @self: a #GtdTaskListView
 *
 * Retrieves the current default date which new tasks are set to.
 *
 * Returns: (nullable): a #GDateTime, or %NULL
 */
GDateTime*
gtd_task_list_view_get_default_date (GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv;

  g_return_val_if_fail (GTD_IS_TASK_LIST_VIEW (self), NULL);

  priv = gtd_task_list_view_get_instance_private (self);

  return priv->default_date;
}

/**
 * gtd_task_list_view_set_default_date:
 * @self: a #GtdTaskListView
 * @default_date: (nullable): the default_date, or %NULL
 *
 * Sets the current default date.
 */
void
gtd_task_list_view_set_default_date   (GtdTaskListView *self,
                                       GDateTime       *default_date)
{
  GtdTaskListViewPrivate *priv;

  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (self));

  priv = gtd_task_list_view_get_instance_private (self);

  if (priv->default_date == default_date)
    return;

  g_clear_pointer (&priv->default_date, g_date_time_unref);
  priv->default_date = default_date ? g_date_time_ref (default_date) : NULL;
}

/**
 * gtd_task_list_view_get_handle_subtasks:
 * @self: a #GtdTaskListView
 *
 * Retirves whether @self handle subtasks, i.e. make the rows
 * change padding depending on their depth, show an arrow button
 * to toggle subtasks, among others.
 *
 * Returns: %TRUE if @self handles subtasks, %FALSE otherwise
 */
gboolean
gtd_task_list_view_get_handle_subtasks (GtdTaskListView *self)
{
  GtdTaskListViewPrivate *priv;

  g_return_val_if_fail (GTD_IS_TASK_LIST_VIEW (self), FALSE);

  priv = gtd_task_list_view_get_instance_private (self);

  return priv->handle_subtasks;
}

/**
 * gtd_task_list_view_set_handle_subtasks:
 * @self: a #GtdTaskListView
 * @handle_subtasks: %TRUE to make @self handle subtasks, %FALSE to disable subtasks.
 *
 * If %TRUE, makes @self handle subtasks, adjust the task rows according to their
 * hierarchy level at the subtask tree and show the arrow button to toggle subtasks
 * of a given task.
 *
 * Drag and drop tasks will only work if @self handles subtasks as well.
 */
void
gtd_task_list_view_set_handle_subtasks (GtdTaskListView *self,
                                        gboolean         handle_subtasks)
{
  GtdTaskListViewPrivate *priv;

  g_return_if_fail (GTD_IS_TASK_LIST_VIEW (self));

  priv = gtd_task_list_view_get_instance_private (self);

  if (priv->handle_subtasks == handle_subtasks)
    return;

  priv->handle_subtasks = handle_subtasks;

  g_object_notify (G_OBJECT (self), "handle-subtasks");
}
