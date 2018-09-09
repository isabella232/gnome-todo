/* gtd-list-store.h
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

#include <glib-object.h>

G_BEGIN_DECLS

#define GTD_TYPE_LIST_STORE (gtd_list_store_get_type())

G_DECLARE_FINAL_TYPE (GtdListStore, gtd_list_store, GTD, LIST_STORE, GObject)

GtdListStore*        gtd_list_store_new                          (GType               item_type);

void                 gtd_list_store_insert                       (GtdListStore       *store,
                                                                  guint               position,
                                                                  gpointer            item);

guint                gtd_list_store_insert_sorted                (GtdListStore       *store,
                                                                  gpointer            item,
                                                                  GCompareDataFunc    compare_func,
                                                                  gpointer            user_data);

void                 gtd_list_store_sort                         (GtdListStore       *store,
                                                                  GCompareDataFunc    compare_func,
                                                                  gpointer            user_data);

void                 gtd_list_store_append                       (GtdListStore       *store,
                                                                  gpointer            item);

void                 gtd_list_store_remove                       (GtdListStore       *store,
                                                                  gpointer            item);

void                 gtd_list_store_remove_at_position           (GtdListStore       *store,
                                                                  guint               position);

void                 gtd_list_store_remove_all                   (GtdListStore       *store);

void                 gtd_list_store_splice                       (GtdListStore       *store,
                                                                  guint               position,
                                                                  guint               n_removals,
                                                                  gpointer           *additions,
                                                                  guint               n_additions);

G_END_DECLS
