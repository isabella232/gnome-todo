/* gtd-widget.c
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

#include "gtd-widget.h"
#include "gtd-rows-common-private.h"

G_DEFINE_TYPE (GtdWidget, gtd_widget, GTK_TYPE_WIDGET)

static void
gtd_widget_dispose (GObject *object)
{
  GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (object));

  while (child)
    {
      GtkWidget *next = gtk_widget_get_next_sibling (child);

      gtk_widget_unparent (child);
      child = next;
    }

  G_OBJECT_CLASS (gtd_widget_parent_class)->dispose (object);
}

static void
gtd_widget_class_init (GtdWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = gtd_widget_dispose;
}

static void
gtd_widget_init (GtdWidget *self)
{
}

GtkWidget*
gtd_widget_new (void)
{
  return g_object_new (GTD_TYPE_WIDGET, NULL);
}
