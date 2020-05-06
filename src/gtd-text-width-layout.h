/* gtd-text-width-layout.h
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

#define GTD_TYPE_TEXT_WIDTH_LAYOUT (gtd_text_width_layout_get_type())
G_DECLARE_FINAL_TYPE (GtdTextWidthLayout, gtd_text_width_layout, GTD, TEXT_WIDTH_LAYOUT, GtkLayoutManager)

GtkLayoutManager*    gtd_text_width_layout_new                   (void);

gint                 gtd_text_width_layout_get_max_width_chars   (GtdTextWidthLayout *self);

void                 gtd_text_width_layout_set_max_width_chars   (GtdTextWidthLayout *self,
                                                                  gint                max_width_chars);

gint                 gtd_text_width_layout_get_width_chars       (GtdTextWidthLayout *self);

void                 gtd_text_width_layout_set_width_chars       (GtdTextWidthLayout *self,
                                                                  gint                width_chars);

G_END_DECLS
