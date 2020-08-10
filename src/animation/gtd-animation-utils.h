/* gtd-animation-utils.h
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

#include <glib-object.h>

G_BEGIN_DECLS


/**
 * GtdProgressFunc:
 * @a: the initial value of an interval
 * @b: the final value of an interval
 * @progress: the progress factor, between 0 and 1
 * @retval: the value used to store the progress
 *
 * Prototype of the progress function used to compute the value
 * between the two ends @a and @b of an interval depending on
 * the value of @progress.
 *
 * The #GValue in @retval is already initialized with the same
 * type as @a and @b.
 *
 * This function will be called by #GtdInterval if the
 * type of the values of the interval was registered using
 * gtd_interval_register_progress_func().
 *
 * Return value: %TRUE if the function successfully computed
 *   the value and stored it inside @retval
 */
typedef gboolean (* GtdProgressFunc) (const GValue *a,
                                      const GValue *b,
                                      gdouble       progress,
                                      GValue       *retval);

void            gtd_interval_register_progress_func (GType           value_type,
                                                     GtdProgressFunc func);


gboolean        gtd_has_progress_function  (GType gtype);
gboolean        gtd_run_progress_function  (GType         gtype,
                                            const GValue *initial,
                                            const GValue *final,
                                            gdouble       progress,
                                            GValue       *retval);

G_END_DECLS
