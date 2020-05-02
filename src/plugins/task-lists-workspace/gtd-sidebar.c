/* gtd-sidebar.c
 *
 * Copyright 2018-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "GtdSidebar"

#include "gtd-debug.h"
#include "gtd-manager.h"
#include "gtd-panel.h"
#include "gtd-provider.h"
#include "gtd-sidebar.h"
#include "gtd-sidebar-list-row.h"
#include "gtd-sidebar-panel-row.h"
#include "gtd-sidebar-provider-row.h"
#include "gtd-task-list.h"
#include "gtd-task-list-panel.h"
#include "gtd-utils.h"
#include "notification/gtd-notification.h"

#include <glib/gi18n.h>

struct _GtdSidebar
{
  GtkBox              parent;

  GtkListBox         *archive_listbox;
  GtkListBoxRow      *archive_row;
  GtkListBox         *listbox;
  GtkStack           *stack;

  GtkStack           *panel_stack;
  GtdPanel           *task_list_panel;

  GSimpleActionGroup *action_group;
};

G_DEFINE_TYPE (GtdSidebar, gtd_sidebar, GTK_TYPE_BOX)


/*
 * Auxiliary methods
 */

static gboolean
activate_row_below (GtdSidebar        *self,
                    GtdSidebarListRow *current_row)
{
  g_autoptr (GList) children = NULL;
  GtkWidget *next_row;
  GtkWidget *parent;
  GList *l;
  gboolean after_deleted;

  parent = gtk_widget_get_parent (GTK_WIDGET (current_row));
  children = gtk_container_get_children (GTK_CONTAINER (parent));
  after_deleted = FALSE;
  next_row = NULL;

  for (l = children; l; l = l->next)
    {
      GtkWidget *row = l->data;

      if (row == (GtkWidget*) current_row)
        {
          after_deleted = TRUE;
          continue;
        }

      if (!gtk_widget_get_visible (row) ||
          !gtk_list_box_row_get_activatable (GTK_LIST_BOX_ROW (row)))
        {
          continue;
        }

      next_row = row;

      if (after_deleted)
        break;
    }

  if (next_row)
    g_signal_emit_by_name (next_row, "activate");

  return next_row != NULL;
}

static void
add_task_list (GtdSidebar  *self,
               GtdTaskList *list)
{
  if (gtd_task_list_is_inbox (list))
    return;

  g_debug ("Adding task list '%s'", gtd_task_list_get_name (list));

  if (!gtd_task_list_get_archived (list))
    {
      gtk_list_box_prepend (self->listbox, gtd_sidebar_list_row_new (list));
      gtk_list_box_invalidate_filter (self->listbox);
    }
  else
    {
      gtk_list_box_prepend (self->archive_listbox, gtd_sidebar_list_row_new (list));
      gtk_list_box_invalidate_filter (self->archive_listbox);
    }
}

static void
add_panel (GtdSidebar *self,
           GtdPanel   *panel)
{
  GtkWidget *row;

  g_debug ("Adding panel '%s'", gtd_panel_get_panel_name (panel));

  row = gtd_sidebar_panel_row_new (panel);

  gtk_list_box_prepend (self->listbox, row);
}

static void
add_provider (GtdSidebar  *self,
              GtdProvider *provider)
{
  g_debug ("Adding provider '%s'", gtd_provider_get_name (provider));

  gtk_list_box_prepend (self->listbox, gtd_sidebar_provider_row_new (provider));
  gtk_list_box_prepend (self->archive_listbox, gtd_sidebar_provider_row_new (provider));
}

static gint
compare_panels (GtdSidebarPanelRow *row_a,
                GtdSidebarPanelRow *row_b)
{
  GtdPanel *panel_a;
  GtdPanel *panel_b;

  panel_a = gtd_sidebar_panel_row_get_panel (row_a);
  panel_b = gtd_sidebar_panel_row_get_panel (row_b);

  return gtd_panel_get_priority (panel_b) - gtd_panel_get_priority (panel_a);
}

static gint
compare_providers (GtdSidebarProviderRow *row_a,
                   GtdSidebarProviderRow *row_b)
{
  GtdProvider *provider_a;
  GtdProvider *provider_b;

  provider_a = gtd_sidebar_provider_row_get_provider (row_a);
  provider_b = gtd_sidebar_provider_row_get_provider (row_b);

  return gtd_provider_compare (provider_a, provider_b);
}

static gint
compare_lists (GtdSidebarListRow *row_a,
               GtdSidebarListRow *row_b)
{
  GtdTaskList *list_a;
  GtdTaskList *list_b;
  gint result;

  list_a = gtd_sidebar_list_row_get_task_list (row_a);
  list_b = gtd_sidebar_list_row_get_task_list (row_b);

  /* First, compare by their providers */
  result = gtd_provider_compare (gtd_task_list_get_provider (list_a), gtd_task_list_get_provider (list_b));

  if (result != 0)
    return result;

  return gtd_collate_compare_strings (gtd_task_list_get_name (list_a), gtd_task_list_get_name (list_b));
}

typedef gpointer (*GetDataFunc) (gpointer data);

static gpointer
get_row_internal (GtdSidebar  *self,
                  GtkListBox  *listbox,
                  GType        type,
                  GetDataFunc  get_data_func,
                  gpointer     data)
{
  g_autoptr (GList) rows = NULL;
  GList *l;

  rows = gtk_container_get_children (GTK_CONTAINER (listbox));

  for (l = rows; l; l = l->next)
    {
      if (g_type_is_a (G_OBJECT_TYPE (l->data), type) && get_data_func (l->data) == data)
          return l->data;
    }

  return NULL;
}

static GtkListBoxRow*
get_row_for_panel (GtdSidebar *self,
                   GtdPanel   *panel)
{
  return get_row_internal (self,
                           self->listbox,
                           GTD_TYPE_SIDEBAR_PANEL_ROW,
                           (GetDataFunc) gtd_sidebar_panel_row_get_panel,
                           panel);
}

static GtkListBoxRow*
get_row_for_provider (GtdSidebar  *self,
                      GtkListBox  *listbox,
                      GtdProvider *provider)
{
  return get_row_internal (self,
                           listbox,
                           GTD_TYPE_SIDEBAR_PROVIDER_ROW,
                           (GetDataFunc) gtd_sidebar_provider_row_get_provider,
                           provider);
}

static GtkListBoxRow*
get_row_for_task_list (GtdSidebar  *self,
                       GtkListBox  *listbox,
                       GtdTaskList *list)
{
  return get_row_internal (self,
                           listbox,
                           GTD_TYPE_SIDEBAR_LIST_ROW,
                           (GetDataFunc) gtd_sidebar_list_row_get_task_list,
                           list);
}

static void
activate_appropriate_row (GtdSidebar    *self,
                          GtkListBoxRow *row)
{
  GtkListBoxRow *to_be_activated;

  if (activate_row_below (self, GTD_SIDEBAR_LIST_ROW (row)))
    return;

  gtk_widget_activate_action (GTK_WIDGET (self),
                              "task-lists-workspace.toggle-archive",
                              "b",
                              FALSE);

  to_be_activated = gtk_list_box_get_row_at_index (self->listbox, 0);
  g_signal_emit_by_name (to_be_activated, "activate");
}

/*
 * Callbacks
 */

static void
on_action_move_up_activated_cb (GSimpleAction *simple,
                                GVariant      *parameters,
                                gpointer       user_data)
{
  GtkListBoxRow *selected_row;
  GtkListBoxRow *previous_row;
  GtdSidebar *self;
  gint selected_row_index;

  GTD_ENTRY;

  self = GTD_SIDEBAR (user_data);
  selected_row = gtk_list_box_get_selected_row (self->listbox);
  g_assert (selected_row != NULL);

  selected_row_index = gtk_list_box_row_get_index (selected_row);
  if (selected_row_index == 0)
    return;

  do
    {
      previous_row = gtk_list_box_get_row_at_index (self->listbox,
                                                    --selected_row_index);
    }
  while (previous_row &&
         (previous_row == self->archive_row ||
          !gtk_list_box_row_get_activatable (previous_row)));


  if (previous_row)
    g_signal_emit_by_name (previous_row, "activate");

  GTD_EXIT;
}

static void
on_action_move_down_activated_cb (GSimpleAction *simple,
                                  GVariant      *parameters,
                                  gpointer       user_data)
{
  GtkListBoxRow *selected_row;
  GtkListBoxRow *next_row;
  GtdSidebar *self;
  gint selected_row_index;

  GTD_ENTRY;

  self = GTD_SIDEBAR (user_data);
  selected_row = gtk_list_box_get_selected_row (self->listbox);
  g_assert (selected_row != NULL);

  selected_row_index = gtk_list_box_row_get_index (selected_row);

  do
    {
      next_row = gtk_list_box_get_row_at_index (self->listbox,
                                                ++selected_row_index);
    }
  while (next_row &&
         (next_row == self->archive_row ||
          !gtk_list_box_row_get_activatable (next_row)));


  if (next_row)
    g_signal_emit_by_name (next_row, "activate");

  GTD_EXIT;
}

static void
on_panel_added_cb (GtdManager *manager,
                   GtdPanel   *panel,
                   GtdSidebar *self)
{
  add_panel (self, panel);
}

static void
on_panel_removed_cb (GtdManager *manager,
                     GtdPanel   *panel,
                     GtdSidebar *self)
{
  GtkListBoxRow *row = get_row_for_panel (self, panel);

  g_debug ("Removing panel '%s'", gtd_panel_get_panel_name (panel));

  if (row)
    gtk_widget_destroy (GTK_WIDGET (row));
}

static void
delete_list_cb (GtdNotification *notification,
                gpointer         user_data)
{
  GtdTaskList *list;
  GtdProvider *provider;

  list = GTD_TASK_LIST (user_data);
  provider = gtd_task_list_get_provider (list);

  g_assert (provider != NULL);
  g_assert (gtd_task_list_is_removable (list));

  gtd_provider_remove_task_list (provider, list);
}

static void
undo_delete_list_cb (GtdNotification *notification,
                     gpointer         user_data)
{
  g_assert (GTD_IS_SIDEBAR_LIST_ROW (user_data));

  gtk_widget_show (GTK_WIDGET (user_data));
}

static void
on_task_list_panel_list_deleted_cb (GtdTaskListPanel *panel,
                                    GtdTaskList      *list,
                                    GtdSidebar       *self)
{
  GtdSidebarListRow *row;
  GtdNotification *notification;
  g_autofree gchar *title = NULL;

  if (gtd_task_list_get_archived (list))
    row = (GtdSidebarListRow*) get_row_for_task_list (self, self->archive_listbox, list);
  else
    row = (GtdSidebarListRow*) get_row_for_task_list (self, self->listbox, list);

  g_assert (row != NULL && GTD_IS_SIDEBAR_LIST_ROW (row));

  GTD_TRACE_MSG ("Removing task list row from sidebar");

  title = g_strdup_printf (_("Task list <b>%s</b> removed"), gtd_task_list_get_name (list));
  notification = gtd_notification_new (title, 6000.0);
  gtd_notification_set_primary_action (notification, delete_list_cb, list);
  gtd_notification_set_secondary_action (notification, _("Undo"), undo_delete_list_cb, row);

  gtd_manager_send_notification (gtd_manager_get_default (), notification);

  /*
   * If the deleted list is selected, go to the next one (or previous, if
   * there are no other task list after this one).
   */
  if (gtk_list_box_row_is_selected (GTK_LIST_BOX_ROW (row)))
    activate_appropriate_row (self, GTK_LIST_BOX_ROW (row));

  gtk_widget_hide (GTK_WIDGET (row));
}

static void
on_listbox_row_activated_cb (GtkListBox    *panels_listbox,
                             GtkListBoxRow *row,
                             GtdSidebar    *self)
{
  if (GTD_IS_SIDEBAR_PANEL_ROW (row))
    {
      GtdPanel *panel = gtd_sidebar_panel_row_get_panel (GTD_SIDEBAR_PANEL_ROW (row));

      gtk_widget_activate_action (GTK_WIDGET (self),
                                  "task-lists-workspace.activate-panel",
                                  "(sv)",
                                  gtd_panel_get_panel_name (panel),
                                  g_variant_new_maybe (G_VARIANT_TYPE_VARIANT, NULL));
    }
  else if (GTD_IS_SIDEBAR_PROVIDER_ROW (row))
    {
      /* Do nothing */
    }
  else if (GTD_IS_SIDEBAR_LIST_ROW (row))
    {
      GVariantBuilder builder;
      GtdProvider *provider;
      GtdTaskList *list;

      list = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row));
      provider = gtd_task_list_get_provider (list);

      g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&builder, "{sv}",
                             "provider-id",
                             g_variant_new_string (gtd_provider_get_id (provider)));
      g_variant_builder_add (&builder, "{sv}",
                             "task-list-id",
                             g_variant_new_string (gtd_object_get_uid (GTD_OBJECT (list))));

      gtk_widget_activate_action (GTK_WIDGET (self),
                                  "task-lists-workspace.activate-panel",
                                  "(sv)",
                                  "task-list-panel",
                                  g_variant_builder_end (&builder));
    }
  else if (row == self->archive_row)
    {
      gtk_widget_activate_action (GTK_WIDGET (self),
                                  "task-lists-workspace.toggle-archive",
                                  "b",
                                  TRUE);
    }
  else
    {
      g_assert_not_reached ();
    }
}

static void
on_panel_stack_visible_child_changed_cb (GtkStack   *panel_stack,
                                         GParamSpec *pspec,
                                         GtdSidebar *self)
{
  GtkListBoxRow *panel_row;
  GtkListBox *listbox;
  GtdPanel *visible_panel;

  g_assert (GTD_IS_PANEL (gtk_stack_get_visible_child (panel_stack)));

  visible_panel = GTD_PANEL (gtk_stack_get_visible_child (panel_stack));
  listbox = self->listbox;

  /*
   * If the currently visible panel is the tasklist panel, we
   * should choose the tasklist that is visible. Otherwise,
   * just select the panel.
   */
  if (visible_panel == self->task_list_panel)
    {
      GtdTaskList *task_list;

      task_list = gtd_task_list_panel_get_task_list (GTD_TASK_LIST_PANEL (self->task_list_panel));
      g_assert (task_list != NULL);

      panel_row = get_row_for_task_list (self, self->listbox, task_list);

      if (!panel_row)
        {
          panel_row = get_row_for_task_list (self, self->archive_listbox, task_list);
          listbox = self->archive_listbox;
        }
    }
  else
    {
      panel_row = get_row_for_panel (self, visible_panel);
    }

  /* Select the row if it's not already selected*/
  if (gtk_list_box_row_is_selected (panel_row))
    gtk_list_box_select_row (listbox, panel_row);
}

static void
on_provider_added_cb (GtdManager  *manager,
                      GtdProvider *provider,
                      GtdSidebar  *self)
{
  add_provider (self, provider);
}

static void
on_provider_removed_cb (GtdManager  *manager,
                        GtdProvider *provider,
                        GtdSidebar  *self)
{
  GtkListBoxRow *row;

  g_debug ("Removing provider '%s'", gtd_provider_get_name (provider));

  row = get_row_for_provider (self, self->listbox, provider);
  gtk_widget_destroy (GTK_WIDGET (row));

  row = get_row_for_provider (self, self->archive_listbox, provider);
  gtk_widget_destroy (GTK_WIDGET (row));
}


static void
on_task_list_added_cb (GtdManager  *manager,
                       GtdTaskList *list,
                       GtdSidebar  *self)
{
  add_task_list (self, list);
}

static void
on_task_list_changed_cb (GtdManager  *manager,
                         GtdTaskList *list,
                         GtdSidebar  *self)
{
  GtkListBoxRow *row;
  GtkListBox *listbox;
  gboolean archived;

  archived = gtd_task_list_get_archived (list);
  listbox = archived ? self->archive_listbox : self->listbox;
  row = get_row_for_task_list (self, listbox, list);

  /*
   * The task was either archived or unarchived; remove it and add to
   * the appropriate listbox.
   */
  if (!row)
    {
      listbox = archived ? self->listbox : self->archive_listbox;
      row = get_row_for_task_list (self, listbox, list);

      if (!row)
        goto out;

      /* Change to another panel or taklist */
      if (gtk_list_box_row_is_selected (row))
        activate_appropriate_row (self, row);

      /* Destroy the old row */
      gtk_widget_destroy (GTK_WIDGET (row));

      /* Add a new row */
      add_task_list (self, list);
    }

out:
  gtk_list_box_invalidate_filter (listbox);
}

static void
on_task_list_removed_cb (GtdManager  *manager,
                         GtdTaskList *list,
                         GtdSidebar  *self)
{
  GtkListBoxRow *row;
  GtkListBox *listbox;

  g_debug ("Removing task list '%s'", gtd_task_list_get_name (list));

  g_assert (!gtd_task_list_is_inbox (list));

  if (!gtd_task_list_get_archived (list))
    listbox = self->listbox;
  else
    listbox = self->archive_listbox;

  row = get_row_for_task_list (self, listbox, list);
  if (!row)
    return;

  gtk_widget_destroy (GTK_WIDGET (row));
  gtk_list_box_invalidate_filter (listbox);
}

static gboolean
filter_archive_listbox_cb (GtkListBoxRow *row,
                           gpointer       user_data)
{
  if (GTD_IS_SIDEBAR_LIST_ROW (row))
    {
      GtdTaskList *list;

      list = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row));
      return gtd_task_list_get_archived (list);
    }
  else if (GTD_IS_SIDEBAR_PROVIDER_ROW (row))
    {
      g_autoptr (GList) lists = NULL;
      GtdProvider *provider;
      GList *l;

      provider = gtd_sidebar_provider_row_get_provider (GTD_SIDEBAR_PROVIDER_ROW (row));
      lists = gtd_provider_get_task_lists (provider);

      for (l = lists; l; l = l->next)
        {
          if (gtd_task_list_get_archived (l->data))
            return TRUE;
        }

      return FALSE;
    }
  else
    {
      g_assert_not_reached ();
    }

  return FALSE;
}

static gboolean
filter_listbox_cb (GtkListBoxRow *row,
                   gpointer       user_data)
{
  GtdTaskList *list;

  if (!GTD_IS_SIDEBAR_LIST_ROW (row))
    return TRUE;

  list = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row));
  return !gtd_task_list_get_archived (list);
}

static gint
sort_listbox_cb (GtkListBoxRow *row_a,
                 GtkListBoxRow *row_b,
                 gpointer       user_data)
{
  GtdSidebar *self = GTD_SIDEBAR (user_data);

  /* Special-case the Archive row */
  if (row_a == self->archive_row || row_b == self->archive_row)
    {
      if (GTD_IS_SIDEBAR_PANEL_ROW (row_b))
        return 1;
      else
        return -1;
    }

  if (G_OBJECT_TYPE (row_a) != G_OBJECT_TYPE (row_b))
    {
      gint result;

      /* Panels go above everything else */
      if (GTD_IS_SIDEBAR_PANEL_ROW (row_b) != GTD_IS_SIDEBAR_PANEL_ROW (row_a))
        return GTD_IS_SIDEBAR_PANEL_ROW (row_b) - GTD_IS_SIDEBAR_PANEL_ROW (row_a);

      /*
       * At this point, we know that row_a and row_b are either provider rows, or
       * tasklist rows. We also know that they're different, i.e. if row_a is a
       * provider row, row_b will be a list one, and vice-versa.
       */
      if (GTD_IS_SIDEBAR_PROVIDER_ROW (row_a))
        {
          GtdProvider *provider_a;
          GtdTaskList *list_b;

          provider_a = gtd_sidebar_provider_row_get_provider (GTD_SIDEBAR_PROVIDER_ROW (row_a));
          list_b = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row_b));

          /*
           * If the providers are different, respect the provider order. If the providers are the
           * same, we must put the provider row above the tasklist row.
           */
          result = gtd_provider_compare (provider_a, gtd_task_list_get_provider (list_b));

          if (result != 0)
            return result;

          return -1;
        }
      else
        {
          GtdTaskList *list_a;
          GtdProvider *provider_b;

          list_a = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row_a));
          provider_b = gtd_sidebar_provider_row_get_provider (GTD_SIDEBAR_PROVIDER_ROW (row_b));

          /* See comment above */
          result = gtd_provider_compare (gtd_task_list_get_provider (list_a), provider_b);

          if (result != 0)
            return result;

          return 1;
        }
    }
  else
    {
      /*
       * We only reach this section of the code if both rows are of the same type,
       * so it doesn't matter which one we get the type from.
       */

      if (GTD_IS_SIDEBAR_PANEL_ROW (row_a))
        return compare_panels (GTD_SIDEBAR_PANEL_ROW (row_a), GTD_SIDEBAR_PANEL_ROW (row_b));

      if (GTD_IS_SIDEBAR_PROVIDER_ROW (row_a))
        return compare_providers (GTD_SIDEBAR_PROVIDER_ROW (row_a), GTD_SIDEBAR_PROVIDER_ROW (row_b));

      if (GTD_IS_SIDEBAR_LIST_ROW (row_a))
        return compare_lists (GTD_SIDEBAR_LIST_ROW (row_a), GTD_SIDEBAR_LIST_ROW (row_b));
    }

  return 0;
}


/*
 * GObject overrides
 */

static void
gtd_sidebar_constructed (GObject *object)
{
  g_autoptr (GList) providers = NULL;
  GListModel *lists;
  GtdManager *manager;
  GtdSidebar *self;
  GList *l;
  guint i;

  self = (GtdSidebar *)object;
  manager = gtd_manager_get_default ();

  G_OBJECT_CLASS (gtd_sidebar_parent_class)->constructed (object);

  /* Add providers */
  providers = gtd_manager_get_providers (manager);

  for (l = providers; l; l = l->next)
    add_provider (self, l->data);

  g_signal_connect (manager, "provider-added", G_CALLBACK (on_provider_added_cb), self);
  g_signal_connect (manager, "provider-removed", G_CALLBACK (on_provider_removed_cb), self);

  /* Add task lists */
  lists = gtd_manager_get_task_lists_model (manager);

  for (i = 0; i < g_list_model_get_n_items (lists); i++)
    {
      g_autoptr (GtdTaskList) list = g_list_model_get_item (lists, i);

      add_task_list (self, list);
    }

  g_signal_connect (manager, "list-added", G_CALLBACK (on_task_list_added_cb), self);
  g_signal_connect (manager, "list-changed", G_CALLBACK (on_task_list_changed_cb), self);
  g_signal_connect (manager, "list-removed", G_CALLBACK (on_task_list_removed_cb), self);
}

static void
gtd_sidebar_class_init (GtdSidebarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = gtd_sidebar_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/task-lists-workspace/gtd-sidebar.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdSidebar, archive_listbox);
  gtk_widget_class_bind_template_child (widget_class, GtdSidebar, archive_row);
  gtk_widget_class_bind_template_child (widget_class, GtdSidebar, listbox);
  gtk_widget_class_bind_template_child (widget_class, GtdSidebar, stack);

  gtk_widget_class_bind_template_callback (widget_class, on_listbox_row_activated_cb);

  gtk_widget_class_set_css_name (widget_class, "sidebar");
}

static void
gtd_sidebar_init (GtdSidebar *self)
{
  static const GActionEntry entries[] = {
    { "move-up", on_action_move_up_activated_cb },
    { "move-down", on_action_move_down_activated_cb },
  };
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_list_box_set_sort_func (self->listbox, sort_listbox_cb, self, NULL);
  gtk_list_box_set_filter_func (self->listbox, filter_listbox_cb, self, NULL);

  gtk_list_box_set_sort_func (self->archive_listbox, sort_listbox_cb, self, NULL);
  gtk_list_box_set_filter_func (self->archive_listbox, filter_archive_listbox_cb, self, NULL);

  self->action_group = g_simple_action_group_new ();

  g_action_map_add_action_entries (G_ACTION_MAP (self->action_group),
                                   entries,
                                   G_N_ELEMENTS (entries),
                                   self);

  gtk_widget_insert_action_group (GTK_WIDGET (self),
                                  "sidebar",
                                  G_ACTION_GROUP (self->action_group));
}

void
gtd_sidebar_set_panel_stack (GtdSidebar *self,
                             GtkStack   *stack)
{
  g_return_if_fail (GTD_IS_SIDEBAR (self));
  g_return_if_fail (GTK_IS_STACK (stack));

  g_assert (self->panel_stack == NULL);

  self->panel_stack = g_object_ref (stack);

  g_signal_connect_object (stack,
                           "notify::visible-child",
                           G_CALLBACK (on_panel_stack_visible_child_changed_cb),
                           self,
                           0);
}


void
gtd_sidebar_set_task_list_panel (GtdSidebar *self,
                                 GtdPanel   *task_list_panel)
{
  g_return_if_fail (GTD_IS_SIDEBAR (self));
  g_return_if_fail (GTD_IS_PANEL (task_list_panel));

  g_assert (self->task_list_panel == NULL);

  self->task_list_panel = g_object_ref (task_list_panel);
  g_signal_connect_object (self->task_list_panel,
                           "list-deleted",
                           G_CALLBACK (on_task_list_panel_list_deleted_cb),
                           self,
                           0);
}

void
gtd_sidebar_activate (GtdSidebar *self)
{
  GtkListBoxRow *first_row;

  g_assert (GTD_IS_SIDEBAR (self));

  first_row = gtk_list_box_get_row_at_index (self->listbox, 0);
  g_signal_emit_by_name (first_row, "activate");
}

void
gtd_sidebar_set_archive_visible (GtdSidebar *self,
                                 gboolean    show_archive)
{
  g_assert (GTD_IS_SIDEBAR (self));

  if (show_archive)
    gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->archive_listbox));
  else
    gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->listbox));
}

void
gtd_sidebar_connect (GtdSidebar *self,
                     GtkWidget  *workspace)
{
  g_signal_connect (workspace, "panel-added", G_CALLBACK (on_panel_added_cb), self);
  g_signal_connect (workspace, "panel-removed", G_CALLBACK (on_panel_removed_cb), self);
}
