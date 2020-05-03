/* gtd-planning-workspace.c
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

#define G_LOG_DOMAIN "GtdPlanningWorkspace"

#include "gtd-planning-workspace.h"

#include "gnome-todo.h"

#include <glib/gi18n.h>

struct _GtdPlanningWorkspace
{
  GtkBin              parent;

  GtkListBox         *planners_listbox;
  GtkWidget          *planners_menu_button;
  GtkStack           *stack;

  PeasExtensionSet   *planners_set;
  GListStore         *planners;
  GtdPlanner         *active_planner;
};

static void          gtd_workspace_iface_init                    (GtdWorkspaceInterface  *iface);

G_DEFINE_TYPE_WITH_CODE (GtdPlanningWorkspace, gtd_planning_workspace, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (GTD_TYPE_WORKSPACE, gtd_workspace_iface_init))

enum
{
  PROP_0,
  PROP_ICON,
  PROP_TITLE,
  N_PROPS,
};


/*
 * Callbacks
 */

static gint
compare_planners_func (gconstpointer a,
                         gconstpointer b,
                         gpointer      user_data)
{
  gint a_priority;
  gint b_priority;

  a_priority = gtd_planner_get_priority ((GtdPlanner *)a);
  b_priority = gtd_planner_get_priority ((GtdPlanner *)b);

  return b_priority - a_priority;
}

static void
on_planner_added_cb (PeasExtensionSet     *extension_set,
                     PeasPluginInfo       *plugin_info,
                     GtdPlanner           *planner,
                     GtdPlanningWorkspace *self)
{
  g_list_store_insert_sorted (self->planners, planner, compare_planners_func, NULL);

  gtk_stack_add_titled (self->stack,
                        GTK_WIDGET (planner),
                        gtd_planner_get_id (planner),
                        gtd_planner_get_title (planner));
}

static void
on_planner_removed_cb (PeasExtensionSet     *extension_set,
                       PeasPluginInfo       *plugin_info,
                       GtdPlanner           *planner,
                       GtdPlanningWorkspace *self)
{
  guint position;

  g_object_ref (planner);

  g_list_store_find (self->planners, planner, &position);
  g_list_store_remove (self->planners, position);

  gtk_container_remove (GTK_CONTAINER (self->stack), GTK_WIDGET (planner));

  g_object_unref (planner);
}

static void
on_stack_visible_child_cb (GtkStack             *stack,
                           GParamSpec           *pspec,
                           GtdPlanningWorkspace *self)
{
  GtdPlanner *planner;
  GtkWidget *visible_child;

  visible_child = gtk_stack_get_visible_child (stack);
  planner = GTD_PLANNER (visible_child);

  if (self->active_planner)
    gtd_planner_deactivate (self->active_planner);

  self->active_planner = planner;

  if (planner)
    {
      g_autoptr (GIcon) icon = gtd_planner_get_icon (planner);

      gtd_planner_activate (planner);
      gtd_menu_button_set_gicon (GTD_MENU_BUTTON (self->planners_menu_button), icon);
    }
}

static GtkWidget*
create_planner_row_func (gpointer item,
                         gpointer user_data)
{
  g_autoptr (GIcon) icon = NULL;
  GtdPlanner *planner;
  GtkWidget *label;
  GtkWidget *image;
  GtkWidget *box;

  planner = GTD_PLANNER (item);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_margin_start (box, 6);
  gtk_widget_set_margin_end (box, 6);
  gtk_widget_set_margin_top (box, 3);
  gtk_widget_set_margin_bottom (box, 3);

  icon = gtd_planner_get_icon (planner);
  image = gtk_image_new_from_gicon (icon);

  label = gtk_label_new (gtd_planner_get_title (planner));
  gtk_widget_set_hexpand (label, TRUE);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);

  gtk_container_add (GTK_CONTAINER (box), image);
  gtk_container_add (GTK_CONTAINER (box), label);

  return box;
}

static void
on_planners_listbox_row_activated_cb (GtkListBox           *listbox,
                                      GtkListBoxRow        *row,
                                      GtdPlanningWorkspace *self)
{
  g_autoptr (GtdPlanner) planner = NULL;

  planner = g_list_model_get_item (G_LIST_MODEL (self->planners),
                                   gtk_list_box_row_get_index (row));

  gtk_stack_set_visible_child (self->stack, GTK_WIDGET (planner));

  gtd_menu_button_popdown (GTD_MENU_BUTTON (self->planners_menu_button));
}

static void
on_show_planner_action_activated_cb (GtkWidget   *widget,
                                     const gchar *action_name,
                                     GVariant    *parameter)
{
  GtdPlanningWorkspace *self;
  const gchar *planner_id;

  self = GTD_PLANNING_WORKSPACE (widget);
  planner_id = g_variant_get_string (parameter, NULL);
  gtk_stack_set_visible_child_name (self->stack, planner_id);
}

/*
 * GtdWorkspace implementation
 */

static const gchar*
gtd_planning_workspace_get_id (GtdWorkspace *workspace)
{
  return "planning";
}

static const gchar*
gtd_planning_workspace_get_title (GtdWorkspace *workspace)
{
  return _("Planning");
}

static gint
gtd_planning_workspace_get_priority (GtdWorkspace *workspace)
{
  return 1500;
}

static GIcon*
gtd_planning_workspace_get_icon (GtdWorkspace *workspace)
{
  return g_themed_icon_new ("document-send-symbolic");
}

static void
gtd_planning_workspace_activate (GtdWorkspace *workspace)
{
  GtdPlanningWorkspace *self = GTD_PLANNING_WORKSPACE (workspace);
  GtdWindow *window = GTD_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));

  gtd_window_embed_widget_in_header (window, self->planners_menu_button, GTK_POS_LEFT);
}

static void
gtd_workspace_iface_init (GtdWorkspaceInterface  *iface)
{
  iface->get_id = gtd_planning_workspace_get_id;
  iface->get_title = gtd_planning_workspace_get_title;
  iface->get_priority = gtd_planning_workspace_get_priority;
  iface->get_icon = gtd_planning_workspace_get_icon;
  iface->activate = gtd_planning_workspace_activate;
}

static void
gtd_planning_workspace_destroy (GtkWidget *widget)
{
  GtdPlanningWorkspace *self = (GtdPlanningWorkspace *)widget;

  g_clear_object (&self->planners_set);

  GTK_WIDGET_CLASS (gtd_planning_workspace_parent_class)->destroy (widget);
}

/*
 * GObject overrides
 */

static void
gtd_planning_workspace_get_property (GObject    *object,
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
gtd_planning_workspace_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void
gtd_planning_workspace_class_init (GtdPlanningWorkspaceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = gtd_planning_workspace_get_property;
  object_class->set_property = gtd_planning_workspace_set_property;

  widget_class->destroy = gtd_planning_workspace_destroy;

  g_object_class_override_property (object_class, PROP_ICON, "icon");
  g_object_class_override_property (object_class, PROP_TITLE, "title");

  g_type_ensure (GTD_TYPE_MENU_BUTTON);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/planning-workspace/gtd-planning-workspace.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdPlanningWorkspace, planners_listbox);
  gtk_widget_class_bind_template_child (widget_class, GtdPlanningWorkspace, planners_menu_button);
  gtk_widget_class_bind_template_child (widget_class, GtdPlanningWorkspace, stack);

  gtk_widget_class_bind_template_callback (widget_class, on_planners_listbox_row_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, on_stack_visible_child_cb);

  gtk_widget_class_install_action (widget_class,
                                   "planning.activate-planner",
                                   "s",
                                   on_show_planner_action_activated_cb);
}

static void
gtd_planning_workspace_init (GtdPlanningWorkspace *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->planners = g_list_store_new (GTD_TYPE_PLANNER);
  self->planners_set = peas_extension_set_new (peas_engine_get_default (),
                                               GTD_TYPE_PLANNER,
                                               NULL);
  peas_extension_set_foreach (self->planners_set,
                              (PeasExtensionSetForeachFunc) on_planner_added_cb,
                              self);

  g_object_connect (self->planners_set,
                    "signal::extension-added", G_CALLBACK (on_planner_added_cb), self,
                    "signal::extension-removed", G_CALLBACK (on_planner_removed_cb), self,
                    NULL);

  gtk_list_box_bind_model (self->planners_listbox,
                           G_LIST_MODEL (self->planners),
                           create_planner_row_func,
                           self,
                           NULL);
}
