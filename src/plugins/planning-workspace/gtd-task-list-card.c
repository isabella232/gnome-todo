/* gtd-task-list-card.c
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

#include "gtd-task-list-card.h"

struct _GtdTaskListCard
{
  GtkFlowBoxChild     parent_instance;

  GtkImage           *color_icon;
  GtkLabel           *title_label;

  GtdTaskList        *task_list;
};

G_DEFINE_TYPE (GtdTaskListCard, gtd_task_list_card, GTK_TYPE_FLOW_BOX_CHILD)

enum
{
  PROP_0,
  PROP_TASK_LIST,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * Auxiliary methods
 */

static void
update_color (GtdTaskListCard *self)
{
  g_autoptr (GdkPaintable) paintable = NULL;
  g_autoptr (GdkRGBA) color = NULL;

  color = gtd_task_list_get_color (self->task_list);
  paintable = gtd_create_circular_paintable (color, 12);

  gtk_image_set_from_paintable (self->color_icon, paintable);
}


/*
 * Callbacks
 */

static void
on_task_list_color_changed_cb (GtdTaskList     *task_list,
                               GParamSpec      *pspec,
                               GtdTaskListCard *self)
{
  update_color (self);
}


/*
 * GObject overrides
 */

static void
gtd_task_list_card_finalize (GObject *object)
{
  GtdTaskListCard *self = (GtdTaskListCard *)object;

  g_clear_object (&self->task_list);

  G_OBJECT_CLASS (gtd_task_list_card_parent_class)->finalize (object);
}

static void
gtd_task_list_card_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GtdTaskListCard *self = GTD_TASK_LIST_CARD (object);

  switch (prop_id)
    {
    case PROP_TASK_LIST:
      g_value_set_object (value, self->task_list);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_list_card_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GtdTaskListCard *self = GTD_TASK_LIST_CARD (object);

  switch (prop_id)
    {
    case PROP_TASK_LIST:
      g_assert (self->task_list == NULL);
      self->task_list = g_value_dup_object (value);

      g_object_bind_property (self->task_list,
                              "name",
                              self->title_label,
                              "label",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

      g_signal_connect (self->task_list, "notify::color", G_CALLBACK (on_task_list_color_changed_cb), self);
      update_color (self);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_list_card_class_init (GtdTaskListCardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_task_list_card_finalize;
  object_class->get_property = gtd_task_list_card_get_property;
  object_class->set_property = gtd_task_list_card_set_property;

  properties[PROP_TASK_LIST] = g_param_spec_object ("task-list",
                                                    "Task list",
                                                    "Task list",
                                                    GTD_TYPE_TASK_LIST,
                                                    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/planning-workspace/gtd-task-list-card.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdTaskListCard, color_icon);
  gtk_widget_class_bind_template_child (widget_class, GtdTaskListCard, title_label);

  gtk_widget_class_set_css_name (widget_class, "task-list-card");
}

static void
gtd_task_list_card_init (GtdTaskListCard *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget*
gtd_task_list_card_new (GtdTaskList *task_list)
{
  return g_object_new (GTD_TYPE_TASK_LIST_CARD,
                       "task-list", task_list,
                       NULL);
}

