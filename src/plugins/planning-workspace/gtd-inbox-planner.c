/* gtd-inbox-planner.c
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

#define G_LOG_DOMAIN "GtdInboxPlanner"

#include "gtd-inbox-planner.h"

#include "gnome-todo.h"

#include "gtd-debug.h"
#include "gtd-provider-section.h"
#include "gtd-task-card.h"

#include <glib/gi18n.h>

struct _GtdInboxPlanner
{
  GtkBin              parent;

  GtkFlowBox         *providers_flowbox;
  GtkFlowBox         *tasks_flowbox;
};

static void          gtd_planner_iface_init                      (GtdPlannerInterface  *iface);

G_DEFINE_TYPE_WITH_CODE (GtdInboxPlanner, gtd_inbox_planner, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (GTD_TYPE_PLANNER, gtd_planner_iface_init))


/*
 * Callbacks
 */

static GtkWidget*
create_provider_section_func (gpointer item,
                              gpointer user_data)
{
  return gtd_provider_section_new (item);
}

static GtkWidget*
create_task_card_func (gpointer item,
                       gpointer user_data)
{
  return gtd_task_card_new (item);
}


/*
 * GtdPlanner implementation
 */

static const gchar*
gtd_inbox_planner_get_id (GtdPlanner *planner)
{
  return "inbox-planner";
}

static const gchar*
gtd_inbox_planner_get_title (GtdPlanner *planner)
{
  return _("Inbox");
}

static gint
gtd_inbox_planner_get_priority (GtdPlanner *planner)
{
  return 1000;
}

static GIcon*
gtd_inbox_planner_get_icon (GtdPlanner *planner)
{
  return g_themed_icon_new ("mail-inbox-symbolic");
}

static void
gtd_planner_iface_init (GtdPlannerInterface *iface)
{
  iface->get_id = gtd_inbox_planner_get_id;
  iface->get_title = gtd_inbox_planner_get_title;
  iface->get_priority = gtd_inbox_planner_get_priority;
  iface->get_icon = gtd_inbox_planner_get_icon;
}

static void
gtd_inbox_planner_finalize (GObject *object)
{
  GtdInboxPlanner *self = (GtdInboxPlanner *)object;

  G_OBJECT_CLASS (gtd_inbox_planner_parent_class)->finalize (object);
}

static void
gtd_inbox_planner_class_init (GtdInboxPlannerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_inbox_planner_finalize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/planning-workspace/gtd-inbox-planner.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdInboxPlanner, providers_flowbox);
  gtk_widget_class_bind_template_child (widget_class, GtdInboxPlanner, tasks_flowbox);

  gtk_widget_class_set_css_name (widget_class, "inbox-planner");
}

static void
gtd_inbox_planner_init (GtdInboxPlanner *self)
{
  g_autoptr (GtkFlattenListModel) flat_model = NULL;
  GtdManager *manager;

  gtk_widget_init_template (GTK_WIDGET (self));

  manager = gtd_manager_get_default ();
  flat_model = gtk_flatten_list_model_new (GTD_TYPE_TASK, gtd_manager_get_inbox_model (manager));
  gtk_flow_box_bind_model (self->tasks_flowbox,
                           G_LIST_MODEL (flat_model),
                           create_task_card_func,
                           self,
                           NULL);

  gtk_flow_box_bind_model (self->providers_flowbox,
                           gtd_manager_get_providers_model (manager),
                           create_provider_section_func,
                           self,
                           NULL);

}
