/* gtd-plugin-today-panel.c
 *
 * Copyright (C) 2016-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#include <gnome-todo.h>

#include "gtd-panel-today.h"
#include "gtd-today-omni-area-addin.h"

G_MODULE_EXPORT void
today_panel_plugin_register_types (PeasObjectModule *module)
{
  peas_object_module_register_extension_type (module,
                                              GTD_TYPE_PANEL,
                                              GTD_TYPE_PANEL_TODAY);

  peas_object_module_register_extension_type (module,
                                              GTD_TYPE_OMNI_AREA_ADDIN,
                                              GTD_TYPE_TODAY_OMNI_AREA_ADDIN);
}
