/* gtd-welcome-action-card.c
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

#include "gtd-welcome-action-card.h"

#include "config.h"

struct _GtdWelcomeActionCard
{
  GtkButton           parent_instance;

  GtkLabel           *counter_label;
  GtkImage           *image;
  GtkLabel           *tasks_label;
  GtkLabel           *title_label;

  guint               counter;
};

G_DEFINE_TYPE (GtdWelcomeActionCard, gtd_welcome_action_card, GTK_TYPE_BUTTON)

enum
{
  PROP_0,
  PROP_ICON_NAME,
  PROP_COUNTER,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * Auxiliary methods
 */

static void
update_counter_label (GtdWelcomeActionCard *self)
{
  g_autofree gchar *text = NULL;

  text = g_strdup_printf ("%u", self->counter);
  gtk_label_set_text (self->counter_label, text);

  if (self->counter == 0)
    gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "empty");

  gtk_label_set_text (self->tasks_label,
                      /* Translators: 'tasks' as in '4 tasks' or '1 task' */
                      g_dngettext (GETTEXT_PACKAGE, "task", "tasks", self->counter));
}


/*
 * GObject overrides
 */

static void
gtd_welcome_action_card_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  GtdWelcomeActionCard *self = GTD_WELCOME_ACTION_CARD (object);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      g_value_set_string (value, gtk_image_get_icon_name (self->image));
      break;

    case PROP_COUNTER:
      g_value_set_uint (value, self->counter);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_text (self->title_label));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_welcome_action_card_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  GtdWelcomeActionCard *self = GTD_WELCOME_ACTION_CARD (object);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      gtk_image_set_from_icon_name (self->image, g_value_get_string (value));
      break;

    case PROP_COUNTER:
      gtd_welcome_action_card_set_counter (self, g_value_get_uint (value));
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title_label, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_welcome_action_card_class_init (GtdWelcomeActionCardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = gtd_welcome_action_card_get_property;
  object_class->set_property = gtd_welcome_action_card_set_property;

  properties[PROP_ICON_NAME] = g_param_spec_string ("icon-name",
                                                    "Icon name",
                                                    "Icon name",
                                                    NULL,
                                                    G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  properties[PROP_COUNTER] = g_param_spec_uint ("counter",
                                                "Counter",
                                                "Counter",
                                                0,
                                                G_MAXUINT,
                                                0,
                                                G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  properties[PROP_TITLE] = g_param_spec_string ("title",
                                                "Title",
                                                "Title",
                                                NULL,
                                                G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/welcome/gtd-welcome-action-card.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdWelcomeActionCard, counter_label);
  gtk_widget_class_bind_template_child (widget_class, GtdWelcomeActionCard, image);
  gtk_widget_class_bind_template_child (widget_class, GtdWelcomeActionCard, tasks_label);
  gtk_widget_class_bind_template_child (widget_class, GtdWelcomeActionCard, title_label);

  gtk_widget_class_set_css_name (widget_class, "welcome-action-card");
}

static void
gtd_welcome_action_card_init (GtdWelcomeActionCard *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  update_counter_label (self);
}

guint
gtd_welcome_action_card_get_counter (GtdWelcomeActionCard *self)
{
  g_return_val_if_fail (GTD_IS_WELCOME_ACTION_CARD (self), 0);

  return self->counter;
}

void
gtd_welcome_action_card_set_counter (GtdWelcomeActionCard *self,
                                     guint                 counter)
{
  g_return_if_fail (GTD_IS_WELCOME_ACTION_CARD (self));

  self->counter = counter;
  update_counter_label (self);
}
