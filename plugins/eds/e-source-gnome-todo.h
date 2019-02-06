/* gtd-task-list-eds.h
 *
 * Copyright (C) 2015 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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
 */

#pragma once

#include <libedataserver/libedataserver.h>

G_BEGIN_DECLS

#define E_SOURCE_EXTENSION_GNOME_TODO "GNOME To Do"

#define E_TYPE_SOURCE_GNOME_TODO (e_source_gnome_todo_get_type())
G_DECLARE_FINAL_TYPE (ESourceGnomeTodo, e_source_gnome_todo, E, SOURCE_GNOME_TODO, ESourceExtension)

guint                e_source_gnome_todo_get_api_version         (ESourceGnomeTodo   *self);

void                 e_source_gnome_todo_set_api_version         (ESourceGnomeTodo   *self,
                                                                  guint               api_version);

G_END_DECLS
