/* gtd-workspace.h
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

#define GTD_TYPE_WORKSPACE (gtd_workspace_get_type ())
G_DECLARE_INTERFACE (GtdWorkspace, gtd_workspace, GTD, WORKSPACE, GtkWidget)

struct _GtdWorkspaceInterface
{
  GTypeInterface parent;

  const gchar*       (*get_id)                                   (GtdWorkspace       *self);

  const gchar*       (*get_title)                                (GtdWorkspace       *self);

  gint               (*get_priority)                             (GtdWorkspace       *self);
};

const gchar*         gtd_workspace_get_id                        (GtdWorkspace       *self);

const gchar*         gtd_workspace_get_title                     (GtdWorkspace       *self);

gint                 gtd_workspace_get_priority                  (GtdWorkspace       *self);

G_END_DECLS
