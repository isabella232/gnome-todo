/* gtd-'gtd-widget.c',.h
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTD_TYPE_WIDGET (gtd_widget_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtdWidget, gtd_widget, GTD, WIDGET, GtkWidget)

struct _GtdWidgetClass
{
  GtkWidgetClass      parent;
};

GtkWidget*           gtd_widget_new                              (void);

void                 gtd_widget_get_pivot_point                  (GtdWidget          *self,
                                                                  graphene_point3d_t *out_pivot_point);

void                 gtd_widget_set_pivot_point                  (GtdWidget                *self,
                                                                  const graphene_point3d_t *pivot_point);

void                 gtd_widget_get_rotation                     (GtdWidget          *self,
                                                                  gfloat             *rotation_x,
                                                                  gfloat             *rotation_y,
                                                                  gfloat             *rotation_z);

void                 gtd_widget_set_rotation                     (GtdWidget          *self,
                                                                  gfloat              rotation_x,
                                                                  gfloat              rotation_y,
                                                                  gfloat              rotation_z);

void                 gtd_widget_get_scale                        (GtdWidget          *self,
                                                                  gfloat             *scale_x,
                                                                  gfloat             *scale_y,
                                                                  gfloat             *scale_z);

void                 gtd_widget_set_scale                        (GtdWidget          *self,
                                                                  gfloat              scale_x,
                                                                  gfloat              scale_y,
                                                                  gfloat              scale_z);

void                 gtd_widget_get_translation                  (GtdWidget          *self,
                                                                  gfloat             *translation_x,
                                                                  gfloat             *translation_y,
                                                                  gfloat             *translation_z);

void                 gtd_widget_set_translation                  (GtdWidget          *self,
                                                                  gfloat              translation_x,
                                                                  gfloat              translation_y,
                                                                  gfloat              translation_z);

GskTransform*        gtd_widget_apply_transform                  (GtdWidget          *self,
                                                                  GskTransform       *transform);

G_END_DECLS
