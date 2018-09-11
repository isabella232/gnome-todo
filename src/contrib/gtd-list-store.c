/* gtd-list-store.c
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

#define G_LOG_DOMAIN "GtdListStore"

#include "gtd-list-store.h"

#include <gio/gio.h>

/**
 * SECTION:gtdliststore
 * @title: GtdListStore
 * @short_description: A simple implementation of #GListModel
 * @include: gio/gio.h
 *
 * #GtdListStore is a simple implementation of #GListModel that stores all
 * items in memory.
 *
 * It provides insertions, deletions, and lookups in logarithmic time
 * with a fast path for the common case of iterating the list linearly.
 */

/**
 * GtdListStore:
 *
 * #GtdListStore is an opaque data structure and can only be accessed
 * using the following functions.
 **/

struct _GtdListStore
{
  GObject             parent;

  GType               item_type;
  GSequence          *items;

  GHashTable         *item_to_iter;

  /* cache */
  guint               last_position;
  GSequenceIter      *last_iter;
};

enum
{
  PROP_0,
  PROP_ITEM_TYPE,
  N_PROPERTIES
};

static void gtd_list_store_iface_init (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtdListStore, gtd_list_store, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, gtd_list_store_iface_init));

static void
remove_item_from_sequence_cb (gpointer item,
                              gpointer user_data)
{
  GSequenceIter *it;
  GtdListStore *store;

  store = (GtdListStore *)user_data;
  it = g_hash_table_lookup (store->item_to_iter, item);

  g_hash_table_remove (store->item_to_iter, item);
  g_sequence_remove (it);
}

static void
gtd_list_store_items_changed (GtdListStore *store,
                              guint         position,
                              guint         removed,
                              guint         added)
{
  /* check if the iter cache may have been invalidated */
  if (position <= store->last_position)
    {
      store->last_iter = NULL;
      store->last_position = -1u;
    }

  g_list_model_items_changed (G_LIST_MODEL (store), position, removed, added);
}

static void
gtd_list_store_dispose (GObject *object)
{
  GtdListStore *store = GTD_LIST_STORE (object);

  g_clear_pointer (&store->item_to_iter, g_hash_table_destroy);
  g_clear_pointer (&store->items, g_sequence_free);

  G_OBJECT_CLASS (gtd_list_store_parent_class)->dispose (object);
}

static void
gtd_list_store_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  GtdListStore *store = GTD_LIST_STORE (object);

  switch (property_id)
    {
    case PROP_ITEM_TYPE:
      g_value_set_gtype (value, store->item_type);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
gtd_list_store_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  GtdListStore *store = GTD_LIST_STORE (object);

  switch (property_id)
    {
    case PROP_ITEM_TYPE: /* construct-only */
      store->item_type = g_value_get_gtype (value);
      if (!g_type_is_a (store->item_type, G_TYPE_OBJECT))
        g_critical ("GtdListStore cannot store items of type '%s'. Items must be GObjects.",
                    g_type_name (store->item_type));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
gtd_list_store_class_init (GtdListStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = gtd_list_store_dispose;
  object_class->get_property = gtd_list_store_get_property;
  object_class->set_property = gtd_list_store_set_property;

  /**
   * GtdListStore:item-type:
   *
   * The type of items contained in this list store. Items must be
   * subclasses of #GObject.
   **/
  g_object_class_install_property (object_class, PROP_ITEM_TYPE,
    g_param_spec_gtype ("item-type", "", "", G_TYPE_OBJECT,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static GType
gtd_list_store_get_item_type (GListModel *list)
{
  GtdListStore *store = GTD_LIST_STORE (list);

  return store->item_type;
}

static guint
gtd_list_store_get_n_items (GListModel *list)
{
  GtdListStore *store = GTD_LIST_STORE (list);

  return g_sequence_get_length (store->items);
}

static gpointer
gtd_list_store_get_item (GListModel *list,
                       guint       position)
{
  GtdListStore *store = GTD_LIST_STORE (list);
  GSequenceIter *it = NULL;

  if (store->last_position != -1u)
    {
      if (store->last_position == position + 1)
        it = g_sequence_iter_prev (store->last_iter);
      else if (store->last_position == position - 1)
        it = g_sequence_iter_next (store->last_iter);
      else if (store->last_position == position)
        it = store->last_iter;
    }

  if (it == NULL)
    it = g_sequence_get_iter_at_pos (store->items, position);

  store->last_iter = it;
  store->last_position = position;

  if (g_sequence_iter_is_end (it))
    return NULL;
  else
    return g_object_ref (g_sequence_get (it));
}

static void
gtd_list_store_iface_init (GListModelInterface *iface)
{
  iface->get_item_type = gtd_list_store_get_item_type;
  iface->get_n_items = gtd_list_store_get_n_items;
  iface->get_item = gtd_list_store_get_item;
}

static void
gtd_list_store_init (GtdListStore *store)
{
  store->item_to_iter = g_hash_table_new (g_direct_hash, g_direct_equal);
  store->items = g_sequence_new (g_object_unref);
  store->last_position = -1u;
}

/**
 * gtd_list_store_new:
 * @item_type: the #GType of items in the list
 *
 * Creates a new #GtdListStore with items of type @item_type. @item_type
 * must be a subclass of #GObject.
 *
 * Returns: a new #GtdListStore
 */
GtdListStore *
gtd_list_store_new (GType item_type)
{
  /* We only allow GObjects as item types right now. This might change
   * in the future.
   */
  g_return_val_if_fail (g_type_is_a (item_type, G_TYPE_OBJECT), NULL);

  return g_object_new (GTD_TYPE_LIST_STORE,
                       "item-type", item_type,
                       NULL);
}

/**
 * gtd_list_store_insert:
 * @store: a #GtdListStore
 * @position: the position at which to insert the new item
 * @item: (type GObject): the new item
 *
 * Inserts @item into @store at @position. @item must be of type
 * #GtdListStore:item-type or derived from it. @position must be smaller
 * than the length of the list, or equal to it to append.
 *
 * This function takes a ref on @item.
 *
 * Use gtd_list_store_splice() to insert multiple items at the same time
 * efficiently.
 */
void
gtd_list_store_insert (GtdListStore *store,
                       guint         position,
                       gpointer      item)
{
  GSequenceIter *new_it;
  GSequenceIter *it;

  g_return_if_fail (GTD_IS_LIST_STORE (store));
  g_return_if_fail (g_type_is_a (G_OBJECT_TYPE (item), store->item_type));
  g_return_if_fail (position <= (guint) g_sequence_get_length (store->items));

  it = g_sequence_get_iter_at_pos (store->items, position);
  new_it = g_sequence_insert_before (it, g_object_ref (item));

  g_hash_table_insert (store->item_to_iter, item, new_it);

  gtd_list_store_items_changed (store, position, 0, 1);
}

/**
 * gtd_list_store_insert_sorted:
 * @store: a #GtdListStore
 * @item: (type GObject): the new item
 * @compare_func: (scope call): pairwise comparison function for sorting
 * @user_data: (closure): user data for @compare_func
 *
 * Inserts @item into @store at a position to be determined by the
 * @compare_func.
 *
 * The list must already be sorted before calling this function or the
 * result is undefined.  Usually you would approach this by only ever
 * inserting items by way of this function.
 *
 * This function takes a ref on @item.
 *
 * Returns: the position at which @item was inserted
 */
guint
gtd_list_store_insert_sorted (GtdListStore     *store,
                              gpointer          item,
                              GCompareDataFunc  compare_func,
                              gpointer          user_data)
{
  GSequenceIter *it;
  guint position;

  g_return_val_if_fail (GTD_IS_LIST_STORE (store), 0);
  g_return_val_if_fail (g_type_is_a (G_OBJECT_TYPE (item), store->item_type), 0);
  g_return_val_if_fail (compare_func != NULL, 0);

  it = g_sequence_insert_sorted (store->items, g_object_ref (item), compare_func, user_data);
  position = g_sequence_iter_get_position (it);

  g_hash_table_insert (store->item_to_iter, item, it);

  gtd_list_store_items_changed (store, position, 0, 1);

  return position;
}

/**
 * gtd_list_store_sort:
 * @store: a #GtdListStore
 * @compare_func: (scope call): pairwise comparison function for sorting
 * @user_data: (closure): user data for @compare_func
 *
 * Sort the items in @store according to @compare_func.
 */
void
gtd_list_store_sort (GtdListStore     *store,
                     GCompareDataFunc  compare_func,
                     gpointer          user_data)
{
  gint n_items;

  g_return_if_fail (GTD_IS_LIST_STORE (store));
  g_return_if_fail (compare_func != NULL);

  g_sequence_sort (store->items, compare_func, user_data);

  n_items = g_sequence_get_length (store->items);
  gtd_list_store_items_changed (store, 0, n_items, n_items);
}

/**
 * gtd_list_store_append:
 * @store: a #GtdListStore
 * @item: (type GObject): the new item
 *
 * Appends @item to @store. @item must be of type #GtdListStore:item-type.
 *
 * This function takes a ref on @item.
 *
 * Use gtd_list_store_splice() to append multiple items at the same time
 * efficiently.
 */
void
gtd_list_store_append (GtdListStore *store,
                       gpointer      item)
{
  GSequenceIter *it;
  guint n_items;

  g_return_if_fail (GTD_IS_LIST_STORE (store));
  g_return_if_fail (g_type_is_a (G_OBJECT_TYPE (item), store->item_type));

  n_items = g_sequence_get_length (store->items);
  it = g_sequence_append (store->items, g_object_ref (item));

  g_hash_table_insert (store->item_to_iter, item, it);

  gtd_list_store_items_changed (store, n_items, 0, 1);
}

/**
 * gtd_list_store_remove:
 * @store: a #GtdListStore
 * @item: the item that is to be removed
 *
 * Removes the item from @store.
 *
 * Use gtd_list_store_splice() to remove multiple items at the same time
 * efficiently.
 */
void
gtd_list_store_remove (GtdListStore *store,
                       gpointer      item)
{
  GSequenceIter *it;
  guint position;

  g_return_if_fail (GTD_IS_LIST_STORE (store));

  it = g_hash_table_lookup (store->item_to_iter, item);
  g_return_if_fail (!g_sequence_iter_is_end (it));

  position = g_sequence_iter_get_position (it);

  g_hash_table_remove (store->item_to_iter, item);
  g_sequence_remove (it);
  gtd_list_store_items_changed (store, position, 1, 0);
}

/**
 * gtd_list_store_remove_at_position:
 * @store: a #GtdListStore
 * @position: the position of the item that is to be removed
 *
 * Removes the item from @store that is at @position. @position must be
 * smaller than the current length of the list.
 *
 * Use gtd_list_store_splice() to remove multiple items at the same time
 * efficiently.
 */
void
gtd_list_store_remove_at_position (GtdListStore *store,
                                   guint         position)
{
  GSequenceIter *it;

  g_return_if_fail (GTD_IS_LIST_STORE (store));

  it = g_sequence_get_iter_at_pos (store->items, position);
  g_return_if_fail (!g_sequence_iter_is_end (it));

  g_hash_table_remove (store->item_to_iter, g_sequence_get (it));
  g_sequence_remove (it);
  gtd_list_store_items_changed (store, position, 1, 0);
}

/**
 * gtd_list_store_remove_all:
 * @store: a #GtdListStore
 *
 * Removes all items from @store.
 */
void
gtd_list_store_remove_all (GtdListStore *store)
{
  guint n_items;

  g_return_if_fail (GTD_IS_LIST_STORE (store));

  n_items = g_sequence_get_length (store->items);
  g_sequence_remove_range (g_sequence_get_begin_iter (store->items),
                           g_sequence_get_end_iter (store->items));
  g_hash_table_remove_all (store->item_to_iter);

  gtd_list_store_items_changed (store, 0, n_items, 0);
}

/**
 * gtd_list_store_splice:
 * @store: a #GtdListStore
 * @position: the position at which to make the change
 * @n_removals: the number of items to remove
 * @additions: (array length=n_additions) (element-type GObject): the items to add
 * @n_additions: the number of items to add
 *
 * Changes @store by removing @n_removals items and adding @n_additions
 * items to it. @additions must contain @n_additions items of type
 * #GtdListStore:item-type.  %NULL is not permitted.
 *
 * This function is more efficient than gtd_list_store_insert() and
 * gtd_list_store_remove(), because it only emits
 * #GListModel::items-changed once for the change.
 *
 * This function takes a ref on each item in @additions.
 *
 * The parameters @position and @n_removals must be correct (ie:
 * @position + @n_removals must be less than or equal to the length of
 * the list at the time this function is called).
 */
void
gtd_list_store_splice (GtdListStore *store,
                       guint         position,
                       guint         n_removals,
                       gpointer     *additions,
                       guint         n_additions)
{
  GSequenceIter *it;
  guint n_items;

  g_return_if_fail (GTD_IS_LIST_STORE (store));
  g_return_if_fail (position + n_removals >= position); /* overflow */

  n_items = g_sequence_get_length (store->items);
  g_return_if_fail (position + n_removals <= n_items);

  it = g_sequence_get_iter_at_pos (store->items, position);

  if (n_removals)
    {
      GSequenceIter *end;

      end = g_sequence_iter_move (it, n_removals);
      g_sequence_foreach_range (it, end, remove_item_from_sequence_cb, store);

      it = end;
    }

  if (n_additions)
    {
      guint i;

      for (i = 0; i < n_additions; i++)
        {
          if G_UNLIKELY (!g_type_is_a (G_OBJECT_TYPE (additions[i]), store->item_type))
            {
              g_critical ("%s: item %d is a %s instead of a %s.  GtdListStore is now in an undefined state.",
                          G_STRFUNC, i, G_OBJECT_TYPE_NAME (additions[i]), g_type_name (store->item_type));
              return;
            }

          it = g_sequence_insert_before (it, g_object_ref (additions[i]));
          g_hash_table_insert (store->item_to_iter, additions[i], it);

          it = g_sequence_iter_next (it);
        }
    }

  gtd_list_store_items_changed (store, position, n_removals, n_additions);
}

/**
 * gtd_list_store_get_item_position:
 * @store: a #GtdListStore
 * @item: the item to retrieve the position
 *
 * Retrieves the position of @items inside @store. It is a programming
 * error to pass an @item that is not contained in @store.
 *
 * Returns: the position of @item in @store.
 */
guint
gtd_list_store_get_item_position (GtdListStore *store,
                                  gpointer      item)
{
  GSequenceIter *iter;

  g_return_val_if_fail (GTD_IS_LIST_STORE (store), 0);

  iter = g_hash_table_lookup (store->item_to_iter, item);
  g_assert (iter != NULL);

  return g_sequence_iter_get_position (iter);
}
