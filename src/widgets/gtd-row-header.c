/* gtd-row-header.c
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

#include "gtd-row-header.h"
#include "gtd-rows-common-private.h"

struct _GtdRowHeader
{
  GtkWidget parent_instance;
};

G_DEFINE_TYPE (GtdRowHeader, gtd_row_header, GTK_TYPE_WIDGET)

static void
gtd_row_header_class_init (GtdRowHeaderClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->measure = gtd_row_measure_with_max;
}

static void
gtd_row_header_init (GtdRowHeader *self)
{
}

GtkWidget*
gtd_row_header_new (void)
{
  return g_object_new (GTD_TYPE_ROW_HEADER,
                       "halign", GTK_ALIGN_CENTER,
                       NULL);
}
