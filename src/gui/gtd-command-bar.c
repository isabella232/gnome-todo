/* gtd-command-bar.c
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

#include "gtd-command-bar.h"

struct _GtdCommandBar
{
  GtdWidget           parent_instance;

  GtkEntry           *entry;
  GtkRevealer        *revealer;
};

G_DEFINE_TYPE (GtdCommandBar, gtd_command_bar, GTD_TYPE_WIDGET)

/*
 * Callbacks
 */

static void
on_revealer_child_revealed_cb (GtkRevealer   *revealer,
                               GParamSpec    *pspec,
                               GtdCommandBar *self)
{
  g_assert (GTD_IS_COMMAND_BAR (self));
  g_assert (GTK_IS_REVEALER (revealer));

  if (gtk_revealer_get_child_revealed (revealer))
    {
      if (!gtk_widget_has_focus (GTK_WIDGET (self->entry)))
        gtk_widget_grab_focus (GTK_WIDGET (self->entry));
    }
  else
    {
      gtk_widget_hide (GTK_WIDGET (self));
    }
}


/*
 * GObject overrides
 */

static void
gtd_command_bar_finalize (GObject *object)
{
  GtdCommandBar *self = (GtdCommandBar *)object;

  G_OBJECT_CLASS (gtd_command_bar_parent_class)->finalize (object);
}

static void
gtd_command_bar_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtdCommandBar *self = GTD_COMMAND_BAR (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_command_bar_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtdCommandBar *self = GTD_COMMAND_BAR (object);

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_command_bar_class_init (GtdCommandBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_command_bar_finalize;
  object_class->get_property = gtd_command_bar_get_property;
  object_class->set_property = gtd_command_bar_set_property;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/gtd-command-bar.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdCommandBar, entry);
  gtk_widget_class_bind_template_child (widget_class, GtdCommandBar, revealer);

  gtk_widget_class_bind_template_callback (widget_class, on_revealer_child_revealed_cb);

  gtk_widget_class_set_css_name (widget_class, "commandbar");
}

static void
gtd_command_bar_init (GtdCommandBar *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

void
gtd_command_bar_reveal (GtdCommandBar *self)
{
  g_return_if_fail (GTD_IS_COMMAND_BAR (self));

  if (!gtk_widget_get_visible (GTK_WIDGET (self)))
    {
      gtk_revealer_set_reveal_child (self->revealer, FALSE);
      gtk_widget_show (GTK_WIDGET (self));
    }

  gtk_revealer_set_reveal_child (self->revealer, TRUE);
  gtk_widget_grab_focus (GTK_WIDGET (self->entry));
}

void
gtd_command_bar_dismiss (GtdCommandBar *self)
{
  g_return_if_fail (GTD_IS_COMMAND_BAR (self));

  gtk_revealer_set_reveal_child (self->revealer, FALSE);
  gtk_entry_buffer_set_text (gtk_entry_get_buffer (self->entry), "", 0);
}
