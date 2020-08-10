/* gtd-animatable.h
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

#pragma once

#include "gtd-types.h"

G_BEGIN_DECLS

#define GTD_TYPE_ANIMATABLE (gtd_animatable_get_type ())
G_DECLARE_INTERFACE (GtdAnimatable, gtd_animatable, GTD, ANIMATABLE, GObject)

/**
 * GtdAnimatableInterface:
 * @find_property: virtual function for retrieving the #GParamSpec of
 *   an animatable property
 * @get_initial_state: virtual function for retrieving the initial
 *   state of an animatable property
 * @set_final_state: virtual function for setting the state of an
 *   animatable property
 * @interpolate_value: virtual function for interpolating the progress
 *   of a property
 * @get_widget: virtual function for getting associated #GtdWidget
 *
 * Since: 1.0
 */
struct _GtdAnimatableInterface
{
  /*< private >*/
  GTypeInterface parent_iface;

  /*< public >*/
  GParamSpec *(* find_property)     (GtdAnimatable *animatable,
                                     const gchar       *property_name);
  void        (* get_initial_state) (GtdAnimatable *animatable,
                                     const gchar       *property_name,
                                     GValue            *value);
  void        (* set_final_state)   (GtdAnimatable *animatable,
                                     const gchar       *property_name,
                                     const GValue      *value);
  gboolean    (* interpolate_value) (GtdAnimatable *animatable,
                                     const gchar       *property_name,
                                     GtdInterval   *interval,
                                     gdouble            progress,
                                     GValue            *value);
  GtdWidget * (* get_widget)      (GtdAnimatable *animatable);
};

GParamSpec *gtd_animatable_find_property     (GtdAnimatable *animatable,
                                              const gchar   *property_name);

void        gtd_animatable_get_initial_state (GtdAnimatable *animatable,
                                              const gchar   *property_name,
                                              GValue        *value);

void        gtd_animatable_set_final_state   (GtdAnimatable *animatable,
                                              const gchar   *property_name,
                                              const GValue  *value);

gboolean    gtd_animatable_interpolate_value (GtdAnimatable *animatable,
                                              const gchar   *property_name,
                                              GtdInterval   *interval,
                                              gdouble        progress,
                                              GValue        *value);

GtdWidget * gtd_animatable_get_widget      (GtdAnimatable *animatable);

G_END_DECLS
