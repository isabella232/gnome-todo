/* gtd-easing.h
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

#include <glib.h>

#include "gtd-animation-enums.h"

G_BEGIN_DECLS

/**
 * GtdEaseFunc:
 * @t: elapsed time
 * @d: total duration
 *
 * Internal type for the easing functions used by Gtd.
 *
 * Return value: the interpolated value, between -1.0 and 2.0
 */
typedef gdouble (* GtdEaseFunc) (gdouble t, gdouble d);

GtdEaseFunc         gtd_get_easing_func_for_mode                 (GtdEaseMode        mode);

const gchar*        gtd_get_easing_name_for_mode                 (GtdEaseMode        mode);

gdouble             gtd_easing_for_mode                          (GtdEaseMode        mode,
                                                                  gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_linear                             (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_quad                            (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_quad                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_quad                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_cubic                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_cubic                          (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_cubic                       (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_quart                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_quart                          (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_quart                       (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_quint                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_quint                          (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_quint                       (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_sine                            (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_sine                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_sine                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_expo                            (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_expo                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_expo                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_circ                            (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_circ                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_circ                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_elastic                         (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_elastic                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_elastic                     (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_back                            (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_back                           (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_back                        (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_bounce                          (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_out_bounce                         (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_in_out_bounce                      (gdouble            t,
                                                                  gdouble            d);

gdouble              gtd_ease_steps_start                        (gdouble            t,
                                                                  gdouble            d,
                                                                  int                steps);

gdouble              gtd_ease_steps_end                          (gdouble            t,
                                                                  gdouble            d,
                                                                  int                steps);

gdouble              gtd_ease_cubic_bezier                       (gdouble            t,
                                                                  gdouble            d,
                                                                  gdouble            x_1,
                                                                  gdouble            y_1,
                                                                  gdouble            x_2,
                                                                  gdouble            y_2);


G_END_DECLS
