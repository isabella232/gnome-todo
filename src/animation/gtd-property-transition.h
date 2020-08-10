/* gtd-property-transition.h
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

#include "gtd-transition.h"
#include "gtd-types.h"

G_BEGIN_DECLS

#define GTD_TYPE_PROPERTY_TRANSITION (gtd_property_transition_get_type())
G_DECLARE_DERIVABLE_TYPE (GtdPropertyTransition, gtd_property_transition, GTD, PROPERTY_TRANSITION, GtdTransition)

/**
 * GtdPropertyTransitionClass:
 *
 * The #GtdPropertyTransitionClass structure
 * contains private data.
 */
struct _GtdPropertyTransitionClass
{
  /*< private >*/
  GtdTransitionClass parent_class;

  gpointer _padding[8];
};

GtdTransition *         gtd_property_transition_new_for_widget      (GtdWidget  *widget,
                                                                     const char *property_name);

GtdTransition *         gtd_property_transition_new                 (const char *property_name);

void                    gtd_property_transition_set_property_name   (GtdPropertyTransition *transition,
                                                                     const char            *property_name);

const char *            gtd_property_transition_get_property_name   (GtdPropertyTransition *transition);

G_END_DECLS
