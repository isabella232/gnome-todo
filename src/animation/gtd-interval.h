/* gtd-interval.h
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

#define GTD_TYPE_INTERVAL (gtd_interval_get_type())
G_DECLARE_DERIVABLE_TYPE (GtdInterval, gtd_interval, GTD, INTERVAL, GInitiallyUnowned)

/**
 * GtdIntervalClass:
 * @validate: virtual function for validating an interval
 *   using a #GParamSpec
 * @compute_value: virtual function for computing the value
 *   inside an interval using an adimensional factor between 0 and 1
 *
 * The #GtdIntervalClass contains only private data.
 *
 * Since: 1.0
 */
struct _GtdIntervalClass
{
  /*< private >*/
  GInitiallyUnownedClass parent_class;

  /*< public >*/
  gboolean (* validate)      (GtdInterval *self,
                              GParamSpec  *pspec);
  gboolean (* compute_value) (GtdInterval *self,
                              gdouble      factor,
                              GValue      *value);

  /*< private >*/
  /* padding for future expansion */
  void (*_gtd_reserved1) (void);
  void (*_gtd_reserved2) (void);
  void (*_gtd_reserved3) (void);
  void (*_gtd_reserved4) (void);
  void (*_gtd_reserved5) (void);
  void (*_gtd_reserved6) (void);
};

GtdInterval*    gtd_interval_new                (GType gtype,
                                                 ...);

GtdInterval*    gtd_interval_new_with_values    (GType         gtype,
                                                 const GValue *initial,
                                                 const GValue *final);


GtdInterval*     gtd_interval_clone              (GtdInterval *self);

GType            gtd_interval_get_value_type     (GtdInterval *self);

void             gtd_interval_set_initial        (GtdInterval *self,
                                                      ...);

void             gtd_interval_set_initial_value  (GtdInterval  *self,
                                                  const GValue *value);

void             gtd_interval_get_initial_value  (GtdInterval *self,
                                                  GValue      *value);

GValue*          gtd_interval_peek_initial_value (GtdInterval *self);

void             gtd_interval_set_final          (GtdInterval *self,
                                                  ...);

void             gtd_interval_set_final_value    (GtdInterval  *self,
                                                  const GValue *value);

void             gtd_interval_get_final_value    (GtdInterval *self,
                                                  GValue      *value);

GValue*          gtd_interval_peek_final_value   (GtdInterval *self);

void             gtd_interval_set_interval       (GtdInterval *self,
                                                  ...);

void             gtd_interval_get_interval       (GtdInterval *self,
                                                  ...);

gboolean         gtd_interval_validate           (GtdInterval *self,
                                                  GParamSpec  *pspec);

gboolean         gtd_interval_compute_value      (GtdInterval *self,
                                                  gdouble      factor,
                                                  GValue      *value);

const GValue*    gtd_interval_compute            (GtdInterval *self,
                                                  gdouble      factor);

gboolean         gtd_interval_is_valid           (GtdInterval *self);


G_END_DECLS
