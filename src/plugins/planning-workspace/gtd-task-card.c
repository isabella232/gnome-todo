/* gtd-task-card.c
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

#define G_LOG_DOMAIN "GtdTaskCard"

#include "gtd-task-card.h"

struct _GtdTaskCard
{
  GtkFlowBoxChild     parent_instance;

  GtkLabel           *title_label;

  GtdTask            *task;
};

G_DEFINE_TYPE (GtdTaskCard, gtd_task_card, GTK_TYPE_FLOW_BOX_CHILD)

enum
{
  PROP_0,
  PROP_TASK,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];



/*
 * GObject overrides
 */

static void
gtd_task_card_finalize (GObject *object)
{
  GtdTaskCard *self = (GtdTaskCard *)object;

  g_clear_object (&self->task);

  G_OBJECT_CLASS (gtd_task_card_parent_class)->finalize (object);
}

static void
gtd_task_card_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  GtdTaskCard *self = GTD_TASK_CARD (object);

  switch (prop_id)
    {
    case PROP_TASK:
      g_value_set_object (value, self->task);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_card_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  GtdTaskCard *self = GTD_TASK_CARD (object);

  switch (prop_id)
    {
    case PROP_TASK:
      g_assert (self->task == NULL);
      self->task = g_value_dup_object (value);

      g_object_bind_property (self->task,
                              "title",
                              self->title_label,
                              "label",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_card_class_init (GtdTaskCardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_task_card_finalize;
  object_class->get_property = gtd_task_card_get_property;
  object_class->set_property = gtd_task_card_set_property;

  properties[PROP_TASK] = g_param_spec_object ("task",
                                               "Task",
                                               "Task",
                                               GTD_TYPE_TASK,
                                               G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  g_type_ensure (GTD_TYPE_STAR_WIDGET);
  g_type_ensure (GTD_TYPE_TEXT_WIDTH_LAYOUT);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/planning-workspace/gtd-task-card.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdTaskCard, title_label);

  gtk_widget_class_set_css_name (widget_class, "task-card");
}

static void
gtd_task_card_init (GtdTaskCard *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_cursor_from_name (GTK_WIDGET (self), "grab");
}

GtkWidget*
gtd_task_card_new (GtdTask *task)
{
  return g_object_new (GTD_TYPE_TASK_CARD,
                       "task", task,
                       NULL);
}

GtdTask*
gtd_task_card_get_task (GtdTaskCard *self)
{
  g_return_val_if_fail (GTD_IS_TASK_CARD (self), NULL);

  return self->task;
}
