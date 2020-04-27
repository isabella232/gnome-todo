/* gtd-workspace.c
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

#include "gtd-workspace.h"

G_DEFINE_INTERFACE (GtdWorkspace, gtd_workspace, G_TYPE_OBJECT)

static void
gtd_workspace_default_init (GtdWorkspaceInterface *iface)
{
}

/**
 * gtd_workspace_get_id:
 * @self: a #GtdWorkspace
 *
 * Retrieves the id of @self.
 *
 * Returns: the id of @self
 */
const gchar*
gtd_workspace_get_id (GtdWorkspace *self)
{
  g_return_val_if_fail (GTD_IS_WORKSPACE (self), NULL);
  g_return_val_if_fail (GTD_WORKSPACE_GET_IFACE (self)->get_id, NULL);

  return GTD_WORKSPACE_GET_IFACE (self)->get_id (self);
}

/**
 * gtd_workspace_get_title:
 * @self: a #GtdWorkspace
 *
 * Retrieves the title of @self.
 *
 * Returns: the title of @self
 */
const gchar*
gtd_workspace_get_title (GtdWorkspace *self)
{
  g_return_val_if_fail (GTD_IS_WORKSPACE (self), NULL);
  g_return_val_if_fail (GTD_WORKSPACE_GET_IFACE (self)->get_title, NULL);

  return GTD_WORKSPACE_GET_IFACE (self)->get_title (self);
}

/**
 * gtd_workspace_get_priority:
 * @self: a #GtdWorkspace
 *
 * Retrieves the priority of @self.
 *
 * Returns: the priority of @self
 */
gint
gtd_workspace_get_priority (GtdWorkspace *self)
{
  g_return_val_if_fail (GTD_IS_WORKSPACE (self), 0);
  g_return_val_if_fail (GTD_WORKSPACE_GET_IFACE (self)->get_priority, 0);

  return GTD_WORKSPACE_GET_IFACE (self)->get_priority (self);
}
