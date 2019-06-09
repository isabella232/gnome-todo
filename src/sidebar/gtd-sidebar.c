/* gtd-sidebar.c
 *
 * Copyright 2018 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

  GtkListBox         *listbox;

  GtkStack           *panel_stack;
  GtdPanel           *task_list_panel;
};

G_DEFINE_TYPE (GtdSidebar, gtd_sidebar, GTK_TYPE_BOX)


/*
 * Auxiliary methods
 */

static void
activate_row_below (GtdSidebar        *self,
                    GtdSidebarListRow *current_row)
{
  g_autoptr (GList) children = NULL;
  GtkWidget *next_row;
  GList *l;
  gboolean after_deleted;

  children = gtk_container_get_children (GTK_CONTAINER (self->listbox));
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
}

static void
add_task_list (GtdSidebar  *self,
               GtdTaskList *list)
{
  GtkWidget *row;

  g_debug ("Adding task list '%s'", gtd_task_list_get_name (list));

  row = gtd_sidebar_list_row_new (list);

  gtk_list_box_prepend (self->listbox, row);
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
  GtkWidget *row;

  g_debug ("Adding provider '%s'", gtd_provider_get_name (provider));

  row = gtd_sidebar_provider_row_new (provider);

  gtk_list_box_prepend (self->listbox, row);
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
                  GType        type,
                  GetDataFunc  get_data_func,
                  gpointer     data)
{
  g_autoptr (GList) rows = NULL;
  GList *l;

  rows = gtk_container_get_children (GTK_CONTAINER (self->listbox));

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
                           GTD_TYPE_SIDEBAR_PANEL_ROW,
                           (GetDataFunc) gtd_sidebar_panel_row_get_panel,
                           panel);
}

static GtkListBoxRow*
get_row_for_provider (GtdSidebar  *self,
                      GtdProvider *provider)
{
  return get_row_internal (self,
                           GTD_TYPE_SIDEBAR_PROVIDER_ROW,
                           (GetDataFunc) gtd_sidebar_provider_row_get_provider,
                           provider);
}

static GtkListBoxRow*
get_row_for_task_list (GtdSidebar  *self,
                       GtdTaskList *list)
{
  return get_row_internal (self,
                           GTD_TYPE_SIDEBAR_LIST_ROW,
                           (GetDataFunc) gtd_sidebar_list_row_get_task_list,
                           list);
}


/*
 * Callbacks
 */

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

  row = (GtdSidebarListRow*) get_row_for_task_list (self, list);
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
    activate_row_below (self, row);

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

      gtk_stack_set_visible_child (self->panel_stack, GTK_WIDGET (panel));
    }
  else if (GTD_IS_SIDEBAR_PROVIDER_ROW (row))
    {
      /* Do nothing */
    }
  else if (GTD_IS_SIDEBAR_LIST_ROW (row))
    {
      GtdTaskList *list = gtd_sidebar_list_row_get_task_list (GTD_SIDEBAR_LIST_ROW (row));

      /*
       * First, update the tasklist. This must be done before changing the
       * stack's visible child, otherwise we hit an assertion failure.
       */
      gtd_task_list_panel_set_task_list (GTD_TASK_LIST_PANEL (self->task_list_panel), list);

      /* Show the task list panel */
      gtk_stack_set_visible_child (self->panel_stack, GTK_WIDGET (self->task_list_panel));
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
  GtdPanel *visible_panel;

  g_assert (GTD_IS_PANEL (gtk_stack_get_visible_child (panel_stack)));

  visible_panel = GTD_PANEL (gtk_stack_get_visible_child (panel_stack));

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

      panel_row = get_row_for_task_list (self, task_list);
    }
  else
    {
      panel_row = get_row_for_panel (self, visible_panel);
    }

  /* Select the row if it's not already selected*/
  if (panel_row != gtk_list_box_get_selected_row (self->listbox))
    gtk_list_box_select_row (self->listbox, panel_row);
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
  GtkListBoxRow *row = get_row_for_provider (self, provider);

  g_debug ("Removing provider '%s'", gtd_provider_get_name (provider));

  g_assert (row != NULL);

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
on_task_list_removed_cb (GtdManager  *manager,
                         GtdTaskList *list,
                         GtdSidebar  *self)
{
  GtkListBoxRow *row = get_row_for_task_list (self, list);

  g_debug ("Removing task list '%s'", gtd_task_list_get_name (list));

  g_assert (row != NULL);

  gtk_widget_destroy (GTK_WIDGET (row));
}

static gint
sort_listbox_cb (GtkListBoxRow *row_a,
                 GtkListBoxRow *row_b,
                 gpointer       user_data)
{
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
  g_autoptr (GList) panels = NULL;
  GListModel *lists;
  GtdManager *manager;
  GtdSidebar *self;
  GList *l;
  guint i;

  self = (GtdSidebar *)object;
  manager = gtd_manager_get_default ();

  G_OBJECT_CLASS (gtd_sidebar_parent_class)->constructed (object);

  /* Add loaded panels */
  panels = gtd_manager_get_panels (manager);

  for (l = panels; l; l = l->next)
    add_panel (self, l->data);

  g_signal_connect (manager, "panel-added", G_CALLBACK (on_panel_added_cb), self);
  g_signal_connect (manager, "panel-removed", G_CALLBACK (on_panel_removed_cb), self);

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
  g_signal_connect (manager, "list-removed", G_CALLBACK (on_task_list_removed_cb), self);
}

static void
gtd_sidebar_class_init (GtdSidebarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = gtd_sidebar_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/sidebar/gtd-sidebar.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdSidebar, listbox);
  gtk_widget_class_bind_template_callback (widget_class, on_listbox_row_activated_cb);

  gtk_widget_class_set_css_name (widget_class, "sidebar");
}

static void
gtd_sidebar_init (GtdSidebar *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_list_box_set_sort_func (self->listbox, sort_listbox_cb, self, NULL);
}

void
gtd_sidebar_set_panel_stack (GtdSidebar *self,
                             GtkStack   *stack)
{
  g_return_if_fail (GTD_IS_SIDEBAR (self));
  g_return_if_fail (GTK_IS_STACK (stack));

  g_assert (self->panel_stack == NULL);

  self->panel_stack = g_object_ref (stack);

  on_panel_stack_visible_child_changed_cb (stack, NULL, self);
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
