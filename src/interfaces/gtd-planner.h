/* gtd-planner.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTD_TYPE_PLANNER (gtd_planner_get_type ())
G_DECLARE_INTERFACE (GtdPlanner, gtd_planner, GTD, PLANNER, GtkWidget)

struct _GtdPlannerInterface
{
  GTypeInterface      parent;

  const gchar*       (*get_id)                                   (GtdPlanner         *self);

  const gchar*       (*get_title)                                (GtdPlanner         *self);

  gint               (*get_priority)                             (GtdPlanner         *self);

  GIcon*             (*get_icon)                                 (GtdPlanner         *self);

  void               (*activate)                                 (GtdPlanner         *self);

  void               (*deactivate)                               (GtdPlanner         *self);
};

const gchar*         gtd_planner_get_id                          (GtdPlanner         *self);

const gchar*         gtd_planner_get_title                       (GtdPlanner         *self);

gint                 gtd_planner_get_priority                    (GtdPlanner         *self);

GIcon*               gtd_planner_get_icon                        (GtdPlanner         *self);

void                 gtd_planner_activate                        (GtdPlanner         *self);

void                 gtd_planner_deactivate                      (GtdPlanner         *self);

G_END_DECLS
