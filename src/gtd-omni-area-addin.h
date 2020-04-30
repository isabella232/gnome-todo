/* gtd-omni-area-addin.h
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

#include "gtd-types.h"

G_BEGIN_DECLS

#define GTD_TYPE_OMNI_AREA_ADDIN (gtd_omni_area_addin_get_type ())
G_DECLARE_INTERFACE (GtdOmniAreaAddin, gtd_omni_area_addin, GTD, OMNI_AREA_ADDIN, GObject)

struct _GtdOmniAreaAddinInterface
{
  GTypeInterface parent;

  void               (*load)                                     (GtdOmniAreaAddin   *self,
                                                                  GtdOmniArea        *omni_area);

  void               (*unload)                                   (GtdOmniAreaAddin   *self,
                                                                  GtdOmniArea        *omni_area);
};

void                 gtd_omni_area_addin_load                    (GtdOmniAreaAddin   *self,
                                                                  GtdOmniArea        *omni_bar);

void                 gtd_omni_area_addin_unload                  (GtdOmniAreaAddin   *self,
                                                                  GtdOmniArea        *omni_bar);

G_END_DECLS
