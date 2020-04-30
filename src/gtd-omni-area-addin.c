/* gtd-omni-area-addin.c
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

#include "gtd-omni-area-addin.h"

#include "gtd-omni-area.h"

G_DEFINE_INTERFACE (GtdOmniAreaAddin, gtd_omni_area_addin, G_TYPE_OBJECT)

static void
gtd_omni_area_addin_default_init (GtdOmniAreaAddinInterface *iface)
{
}

/**
 * gtd_omni_area_addin_load:
 * @self: an #GtdOmniAreaAddin
 * @omni_bar: an #GtdOmniArea
 *
 * Requests that the #GtdOmniAreaAddin initialize, possibly modifying
 * @omni_bar as necessary.
 */
void
gtd_omni_area_addin_load (GtdOmniAreaAddin *self,
                          GtdOmniArea      *omni_bar)
{
  g_return_if_fail (GTD_IS_OMNI_AREA_ADDIN (self));
  g_return_if_fail (GTD_IS_OMNI_AREA (omni_bar));

  if (GTD_OMNI_AREA_ADDIN_GET_IFACE (self)->load)
    GTD_OMNI_AREA_ADDIN_GET_IFACE (self)->load (self, omni_bar);
}

/**
 * gtd_omni_area_addin_unload:
 * @self: an #GtdOmniAreaAddin
 * @omni_bar: an #GtdOmniArea
 *
 * Requests that the #GtdOmniAreaAddin shutdown, possibly modifying
 * @omni_bar as necessary to return it to the original state before
 * the addin was loaded.
 */
void
gtd_omni_area_addin_unload (GtdOmniAreaAddin *self,
                            GtdOmniArea      *omni_bar)
{
  g_return_if_fail (GTD_IS_OMNI_AREA_ADDIN (self));
  g_return_if_fail (GTD_IS_OMNI_AREA (omni_bar));

  if (GTD_OMNI_AREA_ADDIN_GET_IFACE (self)->unload)
    GTD_OMNI_AREA_ADDIN_GET_IFACE (self)->unload (self, omni_bar);
}
