/* gtd-list-model-sort.h
 *
 * Copyright 2018 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#include <gio/gio.h>

G_BEGIN_DECLS

#define GTD_TYPE_LIST_MODEL_SORT (gtd_list_model_sort_get_type())

typedef gboolean (*GtdListModelCompareFunc) (GObject  *a,
                                             GObject  *b,
                                             gpointer  user_data);

G_DECLARE_FINAL_TYPE (GtdListModelSort, gtd_list_model_sort, GTD, LIST_MODEL_SORT, GObject)

GtdListModelSort*    gtd_list_model_sort_new                     (GListModel         *child_model);

GListModel*          gtd_list_model_sort_get_child_model         (GtdListModelSort   *self);

void                 gtd_list_model_sort_invalidate              (GtdListModelSort   *self);

void                 gtd_list_model_sort_set_sort_func           (GtdListModelSort        *self,
                                                                  GtdListModelCompareFunc  compare_func,
                                                                  gpointer                 compare_func_data,
                                                                  GDestroyNotify           compare_func_data_destroy);

G_END_DECLS
