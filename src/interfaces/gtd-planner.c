/* gtd-planner.c
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

#include "gtd-planner.h"

G_DEFINE_INTERFACE (GtdPlanner, gtd_planner, GTK_TYPE_WIDGET)

static void
gtd_planner_default_init (GtdPlannerInterface *iface)
{
}

/**
 * gtd_planner_get_id:
 * @self: a #GtdPlanner
 *
 * Retrieves the id of @self. It is mandatory to implement
 * this.
 *
 * Returns: (transfer none): the id of @self
 */
const gchar*
gtd_planner_get_id (GtdPlanner *self)
{
  g_return_val_if_fail (GTD_IS_PLANNER (self), NULL);
  g_return_val_if_fail (GTD_PLANNER_GET_IFACE (self)->get_id, NULL);

  return GTD_PLANNER_GET_IFACE (self)->get_id (self);
}

/**
 * gtd_planner_get_title:
 * @self: a #GtdPlanner
 *
 * Retrieves the title of @self. It is mandatory to implement
 * this.
 *
 * Returns: the title of @self
 */
const gchar*
gtd_planner_get_title (GtdPlanner *self)
{
  g_return_val_if_fail (GTD_IS_PLANNER (self), NULL);
  g_return_val_if_fail (GTD_PLANNER_GET_IFACE (self)->get_title, NULL);

  return GTD_PLANNER_GET_IFACE (self)->get_title (self);
}

/**
 * gtd_planner_get_priority:
 * @self: a #GtdPlanner
 *
 * Retrieves the priority of @self. It is mandatory to implement
 * this.
 *
 * Returns: the priority of @self
 */
gint
gtd_planner_get_priority (GtdPlanner *self)
{
  g_return_val_if_fail (GTD_IS_PLANNER (self), 0);
  g_return_val_if_fail (GTD_PLANNER_GET_IFACE (self)->get_priority, 0);

  return GTD_PLANNER_GET_IFACE (self)->get_priority (self);
}

/**
 * gtd_planner_get_priority:
 * @self: a #GtdPlanner
 *
 * Retrieves the icon of @self. It is mandatory to implement
 * this.
 *
 * Returns: (transfer full): a #GIcon
 */
GIcon*
gtd_planner_get_icon (GtdPlanner *self)
{
  g_return_val_if_fail (GTD_IS_PLANNER (self), NULL);
  g_return_val_if_fail (GTD_PLANNER_GET_IFACE (self)->get_icon, NULL);

  return GTD_PLANNER_GET_IFACE (self)->get_icon (self);
}

/**
 * gtd_planner_activate:
 * @self: a #GtdPlanner
 *
 * Activates @self. This happens when the planner
 * becomes active in the main planner.
 */
void
gtd_planner_activate (GtdPlanner *self)
{
  g_return_if_fail (GTD_IS_PLANNER (self));

  if (GTD_PLANNER_GET_IFACE (self)->activate)
    GTD_PLANNER_GET_IFACE (self)->activate (self);
}

/**
 * gtd_planner_deactivate:
 * @self: a #GtdPlanner
 *
 * Deactivates @self. This happens when the planner
 * is switched away in the planner.
 */
void
gtd_planner_deactivate (GtdPlanner *self)
{
  g_return_if_fail (GTD_IS_PLANNER (self));

  if (GTD_PLANNER_GET_IFACE (self)->deactivate)
    GTD_PLANNER_GET_IFACE (self)->deactivate (self);
}

