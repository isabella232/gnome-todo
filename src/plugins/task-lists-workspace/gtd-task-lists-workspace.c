/* gtd-task-lists-workspace.c
 *
 * Copyright 2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#define G_LOG_DOMAIN "GtdTaskListsWorkspace"

#include "gtd-task-lists-workspace.h"

#include "task-lists-workspace.h"
#include "gtd-sidebar.h"
#include "gtd-task-list-panel.h"

#include <libpeas/peas.h>
#include <glib/gi18n.h>

struct _GtdTaskListsWorkspace
{
  GtkBox              parent;

  GtkWidget          *back_button;
  GtkWidget          *end_box;
  GtkMenuButton      *gear_menu_button;
  GtkWidget          *new_list_button;
  GtkBox             *panel_box_end;
  GtkBox             *panel_box_start;
  GtkStack           *stack;
  GtkWidget          *start_box;
  GtdSidebar         *sidebar;
  GtkWidget          *toggle_sidebar_button;

  GtdPanel           *active_panel;
  GtdPanel           *task_list_panel;

  PeasExtensionSet   *panels_set;
  GSimpleActionGroup *action_group;
};

static void          gtd_workspace_iface_init                    (GtdWorkspaceInterface  *iface);

G_DEFINE_TYPE_WITH_CODE (GtdTaskListsWorkspace, gtd_task_lists_workspace, GTK_TYPE_BOX,
                         G_IMPLEMENT_INTERFACE (GTD_TYPE_WORKSPACE, gtd_workspace_iface_init))

enum
{
  PROP_0,
  PROP_ICON,
  PROP_TITLE,
  N_PROPS
};

enum
{
  PANEL_ADDED,
  PANEL_REMOVED,
  NUM_SIGNALS
};

static guint signals[NUM_SIGNALS] = { 0, };


/*
 * Auxiliary methods
 */

static void
add_widgets (GtdTaskListsWorkspace *self,
             GList                 *widgets)
{
  GList *l;

  for (l = widgets; l; l = l->next)
    {
      switch (gtk_widget_get_halign (l->data))
        {
        case GTK_ALIGN_END:
          gtk_box_append (self->panel_box_end, l->data);
          break;

        case GTK_ALIGN_START:
        case GTK_ALIGN_BASELINE:
        case GTK_ALIGN_FILL:
        default:
          gtk_box_append (self->panel_box_start, l->data);
          break;
        }
    }
}

static void
remove_widgets (GtdTaskListsWorkspace *self,
                GList                 *widgets)
{
  GList *l;

  for (l = widgets; l; l = l->next)
    {
      GtkBox *box;

      if (gtk_widget_get_halign (l->data) == GTK_ALIGN_END)
        box = self->panel_box_end;
      else
        box = self->panel_box_start;

      g_object_ref (l->data);
      gtk_box_remove (box, l->data);
    }
}

static void
update_panel_menu (GtdTaskListsWorkspace *self)
{
  GtkPopover *popover;
  const GMenu *menu;

  popover = gtd_panel_get_popover (self->active_panel);
  menu = gtd_panel_get_menu (self->active_panel);

  gtk_widget_set_visible (GTK_WIDGET (self->gear_menu_button), popover || menu);

  if (popover)
    {
      gtk_menu_button_set_popover (self->gear_menu_button, GTK_WIDGET (popover));
    }
  else
    {
      gtk_menu_button_set_popover (self->gear_menu_button, NULL);
      gtk_menu_button_set_menu_model (self->gear_menu_button, G_MENU_MODEL (menu));
    }
}


/*
 * Callbacks
 */

static void
on_action_activate_panel_activated_cb (GSimpleAction *simple,
                                       GVariant      *parameters,
                                       gpointer       user_data)
{
  GtdTaskListsWorkspace *self;
  g_autoptr (GVariant) panel_parameters = NULL;
  g_autofree gchar *panel_id = NULL;
  GtdPanel *panel;

  self = GTD_TASK_LISTS_WORKSPACE (user_data);

  g_variant_get (parameters,
                 "(sv)",
                 &panel_id,
                 &panel_parameters);

  g_debug ("Activating panel '%s'", panel_id);

  panel = (GtdPanel *) gtk_stack_get_child_by_name (self->stack, panel_id);
  g_return_if_fail (panel && GTD_IS_PANEL (panel));

  gtd_panel_activate (panel, panel_parameters);

  gtk_stack_set_visible_child (self->stack, GTK_WIDGET (panel));
}

static void
on_action_toggle_archive_activated_cb (GSimpleAction *simple,
                                       GVariant      *state,
                                       gpointer       user_data)
{
  GtdTaskListsWorkspace *self;
  gboolean archive_visible;

  self = GTD_TASK_LISTS_WORKSPACE (user_data);
  archive_visible = g_variant_get_boolean (state);

  gtk_widget_set_visible (self->new_list_button, !archive_visible);
  gtd_sidebar_set_archive_visible (self->sidebar, archive_visible);
}

static void
on_back_button_clicked_cb (GtkButton             *button,
                           GtdTaskListsWorkspace *self)
{
  gtk_widget_activate_action (GTK_WIDGET (self),
                              "task-lists-workspace.toggle-archive",
                              "b",
                              FALSE);
}

static void
on_panel_added_cb (PeasExtensionSet      *extension_set,
                   PeasPluginInfo        *plugin_info,
                   GtdPanel              *panel,
                   GtdTaskListsWorkspace *self)
{
  gtk_stack_add_titled (self->stack,
                        GTK_WIDGET (g_object_ref_sink (panel)),
                        gtd_panel_get_panel_name (panel),
                        gtd_panel_get_panel_title (panel));

  g_signal_emit (self, signals[PANEL_ADDED], 0, panel);
}

static void
on_panel_removed_cb (PeasExtensionSet      *extension_set,
                     PeasPluginInfo        *plugin_info,
                     GtdPanel              *panel,
                     GtdTaskListsWorkspace *self)
{
  g_object_ref (panel);

  gtk_stack_remove (self->stack, GTK_WIDGET (panel));
  g_signal_emit (self, signals[PANEL_REMOVED], 0, panel);

  g_object_unref (panel);
}

static void
on_panel_menu_changed_cb (GObject               *object,
                          GParamSpec            *pspec,
                          GtdTaskListsWorkspace *self)
{
  if (GTD_PANEL (object) != self->active_panel)
    return;

  update_panel_menu (self);
}
static void
on_stack_visible_child_cb (GtdTaskListsWorkspace *self,
                           GParamSpec            *pspec,
                           GtkStack              *stack)
{
  GtkWidget *visible_child;
  GtdPanel *panel;
  GList *header_widgets;

  visible_child = gtk_stack_get_visible_child (stack);
  panel = GTD_PANEL (visible_child);

  /* Remove previous panel's widgets */
  if (self->active_panel)
    {
      header_widgets = gtd_panel_get_header_widgets (self->active_panel);

      /* Disconnect signals */
      g_signal_handlers_disconnect_by_func (self->active_panel,
                                            on_panel_menu_changed_cb,
                                            self);

      remove_widgets (self, header_widgets);

      g_list_free (header_widgets);
    }

  /* Add current panel's header widgets */
  header_widgets = gtd_panel_get_header_widgets (panel);
  add_widgets (self, header_widgets);

  g_list_free (header_widgets);

  g_signal_connect (panel, "notify::menu", G_CALLBACK (on_panel_menu_changed_cb), self);

  /* Set panel as the new active panel */
  g_set_object (&self->active_panel, panel);

  /* Setup the panel's menu */
  update_panel_menu (self);
}


/*
 * GtdWorkspace implementation
 */

static const gchar*
gtd_task_lists_workspace_get_id (GtdWorkspace *workspace)
{
  return "task-lists";
}

static const gchar*
gtd_task_lists_workspace_get_title (GtdWorkspace *workspace)
{
  return _("Task Lists");
}

static gint
gtd_task_lists_workspace_get_priority (GtdWorkspace *workspace)
{
  return 1000;
}

static GIcon*
gtd_task_lists_workspace_get_icon (GtdWorkspace *workspace)
{
  return g_themed_icon_new ("view-list-symbolic");
}

static void
gtd_task_lists_workspace_activate (GtdWorkspace *workspace)
{
  GtdTaskListsWorkspace *self = GTD_TASK_LISTS_WORKSPACE (workspace);
  GtdWindow *window = GTD_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));

  gtd_window_embed_widget_in_header (window, self->start_box, GTK_POS_LEFT);
  gtd_window_embed_widget_in_header (window, self->end_box, GTK_POS_RIGHT);

  gtd_sidebar_activate (self->sidebar);
}

static void
gtd_workspace_iface_init (GtdWorkspaceInterface  *iface)
{
  iface->get_id = gtd_task_lists_workspace_get_id;
  iface->get_title = gtd_task_lists_workspace_get_title;
  iface->get_priority = gtd_task_lists_workspace_get_priority;
  iface->get_icon = gtd_task_lists_workspace_get_icon;
  iface->activate = gtd_task_lists_workspace_activate;
}


/*
 * GObject overrides
 */

static void
gtd_task_lists_workspace_dispose (GObject *object)
{
  GtdTaskListsWorkspace *self = (GtdTaskListsWorkspace *)object;

  G_OBJECT_CLASS (gtd_task_lists_workspace_parent_class)->dispose (object);

  g_signal_handlers_disconnect_by_func (self->panels_set, on_panel_added_cb, self);
  g_signal_handlers_disconnect_by_func (self->panels_set, on_panel_removed_cb, self);
  g_clear_object (&self->panels_set);
}

static void
gtd_task_lists_workspace_constructed (GObject *object)
{
  GtdTaskListsWorkspace *self = (GtdTaskListsWorkspace *)object;
  GtdManager *manager;

  self = GTD_TASK_LISTS_WORKSPACE (object);

  G_OBJECT_CLASS (gtd_task_lists_workspace_parent_class)->constructed (object);

  /* Add plugins' header widgets, and setup for new plugins */
  manager = gtd_manager_get_default ();

  /* Add loaded panels */
  self->panels_set = peas_extension_set_new (peas_engine_get_default (),
                                             GTD_TYPE_PANEL,
                                             NULL);

  peas_extension_set_foreach (self->panels_set,
                              (PeasExtensionSetForeachFunc) on_panel_added_cb,
                              self);

  g_signal_connect (self->panels_set, "extension-added", G_CALLBACK (on_panel_added_cb), self);
  g_signal_connect (self->panels_set, "extension-removed", G_CALLBACK (on_panel_removed_cb), self);

  g_settings_bind (gtd_manager_get_settings (manager),
                   "sidebar-revealed",
                   self->toggle_sidebar_button,
                   "active",
                   G_SETTINGS_BIND_DEFAULT);
}

static void
gtd_task_lists_workspace_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  GtdWorkspace *workspace = GTD_WORKSPACE (object);

  switch (prop_id)
    {
    case PROP_ICON:
      g_value_take_object (value, gtd_workspace_get_icon (workspace));
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtd_workspace_get_title (workspace));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_lists_workspace_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void
gtd_task_lists_workspace_class_init (GtdTaskListsWorkspaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_resources_register (task_lists_workspace_get_resource ());

  object_class->dispose = gtd_task_lists_workspace_dispose;
  object_class->constructed = gtd_task_lists_workspace_constructed;
  object_class->get_property = gtd_task_lists_workspace_get_property;
  object_class->set_property = gtd_task_lists_workspace_set_property;


  /**
   * GtdTaskListsWorkspace::panel-added:
   * @manager: a #GtdManager
   * @panel: a #GtdPanel
   *
   * The ::panel-added signal is emmited after a #GtdPanel
   * is added.
   */
  signals[PANEL_ADDED] = g_signal_new ("panel-added",
                                        GTD_TYPE_TASK_LISTS_WORKSPACE,
                                        G_SIGNAL_RUN_LAST,
                                        0,
                                        NULL,
                                        NULL,
                                        NULL,
                                        G_TYPE_NONE,
                                        1,
                                        GTD_TYPE_PANEL);

  /**
   * GtdTaskListsWorkspace::panel-removed:
   * @manager: a #GtdManager
   * @panel: a #GtdPanel
   *
   * The ::panel-removed signal is emmited after a #GtdPanel
   * is removed from the list.
   */
  signals[PANEL_REMOVED] = g_signal_new ("panel-removed",
                                         GTD_TYPE_TASK_LISTS_WORKSPACE,
                                         G_SIGNAL_RUN_LAST,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         G_TYPE_NONE,
                                         1,
                                         GTD_TYPE_PANEL);

  g_object_class_override_property (object_class, PROP_ICON, "icon");
  g_object_class_override_property (object_class, PROP_TITLE, "title");

  g_type_ensure (GTD_TYPE_PROVIDER_POPOVER);
  g_type_ensure (GTD_TYPE_SIDEBAR);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/task-lists-workspace/gtd-task-lists-workspace.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, back_button);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, end_box);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, gear_menu_button);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, new_list_button);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, panel_box_end);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, panel_box_start);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, sidebar);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, stack);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, start_box);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListsWorkspace, toggle_sidebar_button);

  gtk_widget_class_bind_template_callback (widget_class, on_back_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_stack_visible_child_cb);
}

static void
gtd_task_lists_workspace_init (GtdTaskListsWorkspace *self)
{
  static const GActionEntry entries[] = {
    { "activate-panel", on_action_activate_panel_activated_cb, "(sv)" },
    { "toggle-archive", on_action_toggle_archive_activated_cb, "b" },
  };

  gtk_widget_init_template (GTK_WIDGET (self));

  self->action_group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (self->action_group),
                                   entries,
                                   G_N_ELEMENTS (entries),
                                   self);
  gtk_widget_insert_action_group (GTK_WIDGET (self),
                                  "task-lists-workspace",
                                  G_ACTION_GROUP (self->action_group));

  gtk_actionable_set_action_target_value (GTK_ACTIONABLE (self->back_button),
                                          g_variant_new_boolean (FALSE));

  /* Task list panel */
  self->task_list_panel = GTD_PANEL (gtd_task_list_panel_new ());
  on_panel_added_cb (NULL, NULL, self->task_list_panel, self);

  gtd_sidebar_connect (self->sidebar, GTK_WIDGET (self));
  gtd_sidebar_set_panel_stack (self->sidebar, self->stack);
  gtd_sidebar_set_task_list_panel (self->sidebar, self->task_list_panel);

}

GtdWorkspace*
gtd_task_lists_workspace_new (void)
{
  return g_object_new (GTD_TYPE_TASK_LISTS_WORKSPACE, NULL);
}
