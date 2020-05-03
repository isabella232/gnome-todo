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

G_DEFINE_INTERFACE (GtdWorkspace, gtd_workspace, GTK_TYPE_WIDGET)

static void
gtd_workspace_default_init (GtdWorkspaceInterface *iface)
{
  /**
   * GtdWorkspace::icon:
   *
   * The icon of the panel.
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_object ("icon",
                                                            "Icon of the workspace",
                                                            "The icon of the workspace",
                                                            G_TYPE_ICON,
                                                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  /**
   * GtdWorkspace::title:
   *
   * The user-visible title of the workspace.
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_string ("title",
                                                            "The title of the workspace",
                                                            "The title of the workspace",
                                                            NULL,
                                                            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
}

/**
 * gtd_workspace_get_id:
 * @self: a #GtdWorkspace
 *
 * Retrieves the id of @self. It is mandatory to implement
 * this.
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
 * Retrieves the title of @self. It is mandatory to implement
 * this.
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
 * Retrieves the priority of @self. It is mandatory to implement
 * this.
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

/**
 * gtd_workspace_get_priority:
 * @self: a #GtdWorkspace
 *
 * Retrieves the icon of @self. It is mandatory to implement
 * this.
 *
 * Returns: (transfer full): a #GIcon
 */
GIcon*
gtd_workspace_get_icon (GtdWorkspace *self)
{
  g_return_val_if_fail (GTD_IS_WORKSPACE (self), NULL);
  g_return_val_if_fail (GTD_WORKSPACE_GET_IFACE (self)->get_icon, NULL);

  return GTD_WORKSPACE_GET_IFACE (self)->get_icon (self);
}

/**
 * gtd_workspace_activate:
 * @self: a #GtdWorkspace
 *
 * Activates @self. This happens when the workspace
 * becomes the active workspace in the main window.
 */
void
gtd_workspace_activate (GtdWorkspace *self)
{
  g_return_if_fail (GTD_IS_WORKSPACE (self));

  if (GTD_WORKSPACE_GET_IFACE (self)->activate)
    GTD_WORKSPACE_GET_IFACE (self)->activate (self);
}

/**
 * gtd_workspace_deactivate:
 * @self: a #GtdWorkspace
 *
 * Deactivates @self. This happens when the workspace
 * is switched away in the main window.
 */
void
gtd_workspace_deactivate (GtdWorkspace *self)
{
  g_return_if_fail (GTD_IS_WORKSPACE (self));

  if (GTD_WORKSPACE_GET_IFACE (self)->deactivate)
    GTD_WORKSPACE_GET_IFACE (self)->deactivate (self);
}

