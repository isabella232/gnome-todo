/* gtd-list-model-sort.c
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

#define G_LOG_DOMAIN "GtdListModelSort"

#include "gtd-debug.h"
#include "gtd-list-model-sort.h"
#include "gtd-task.h"
#include "gtd-task-list.h"

struct _GtdListModelSort
{
  GObject             parent;

  GListModel         *child_model;

  GSequence          *child_seq;
  GSequence          *sort_seq;

  GtdListModelCompareFunc compare_func;
  gpointer            compare_func_data;
  GDestroyNotify      compare_func_data_destroy;

  gboolean            supress_items_changed : 1;

  /* cache */
  gint64              length;
  gint64              last_position;
  GSequenceIter      *last_iter;
};

static void list_model_iface_init (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtdListModelSort, gtd_list_model_sort, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

enum
{
  PROP_0,
  PROP_CHILD_MODEL,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
gtd_list_model_sort_item_free (gpointer data)
{
  GSequenceIter *iter = data;

  g_clear_pointer (&iter, g_sequence_remove);
}

static gint
seq_compare_func (gconstpointer a,
                  gconstpointer b,
                  gpointer      user_data)
{
  GtdListModelSort *self = (GtdListModelSort*) user_data;

  return self->compare_func (G_OBJECT (a), G_OBJECT (b), self->compare_func_data);
}

static gint
default_compare_func (GObject  *a,
                      GObject  *b,
                      gpointer  user_data)
{
  return 0;
}


static void
invalidate_cache (GtdListModelSort *self)
{
  GTD_TRACE_MSG ("Invalidating cache");

  self->last_iter = NULL;
  self->last_position = -1;
}

static void
emit_items_changed (GtdListModelSort *self,
                    guint             position,
                    guint             n_removed,
                    guint             n_added)
{
  if (position <= self->last_position)
    invalidate_cache (self);

  self->length -= n_removed;
  self->length += n_added;

  GTD_TRACE_MSG ("Emitting items-changed(%u, %u, %u)", position, n_removed, n_added);

  g_list_model_items_changed (G_LIST_MODEL (self), position, n_removed, n_added);
}


static void
child_model_items_changed (GtdListModelSort *self,
                           guint             position,
                           guint             n_removed,
                           guint             n_added,
                           GListModel       *child_model)
{
  guint i;

  GTD_ENTRY;

  g_assert (GTD_IS_LIST_MODEL_SORT (self));
  g_assert (G_IS_LIST_MODEL (child_model));
  g_assert (self->child_model == child_model);
  g_assert (position <= (guint)g_sequence_get_length (self->child_seq));
  g_assert (g_sequence_get_length (self->child_seq) - n_removed + n_added == g_list_model_get_n_items (child_model));

  GTD_TRACE_MSG ("Received items-changed(%u, %u, %u)", position, n_removed, n_added);

  if (n_removed > 0)
    {
      GSequenceIter *iter;
      gint64 previous_sort_position = -1u;
      gint64 first_position = -1u;
      gint64 counter = 0;

      /* Small shortcut when all items are removed */
      if (n_removed == (guint)g_sequence_get_length (self->child_seq))
        {
          g_sequence_remove_range (g_sequence_get_begin_iter (self->child_seq),
                                   g_sequence_get_end_iter (self->child_seq));

          g_assert (g_sequence_is_empty (self->child_seq));
          g_assert (g_sequence_is_empty (self->sort_seq));

          emit_items_changed (self, 0, n_removed, 0);

          goto add_new_items;
        }

      iter = g_sequence_get_iter_at_pos (self->child_seq, position);
      g_assert (!g_sequence_iter_is_end (iter));

      for (i = 0; i < n_removed; i++)
        {
          GSequenceIter *sort_iter = g_sequence_get (iter);
          GSequenceIter *to_remove = iter;
          guint sort_position;

          g_assert (g_sequence_iter_get_sequence (sort_iter) == self->sort_seq);

          sort_position = g_sequence_iter_get_position (sort_iter);

          /* Fetch the next while the iter is still valid */
          iter = g_sequence_iter_next (iter);

          /* Cascades into also removing from sort_seq. */
          g_sequence_remove (to_remove);

          if (first_position == -1)
            first_position = sort_position;

          /*
           * This happens when the position in the sorted sequence is different
           * from the position in the child sequence. We try to accumulate as many
           * items-changed() signals as possible, but we can't do that when the
           * order doesn't match.
           */
          if (previous_sort_position != -1 && sort_position != previous_sort_position + 1)
            {
              emit_items_changed (self, first_position, 0, counter);

              first_position = sort_position;
              counter = 0;
            }

          previous_sort_position = sort_position;
          counter++;
        }

      /*
       * Last items-changed() - if the child model is already sorted,
       * only this one will be fired.
       */
      if (counter > 0)
        emit_items_changed (self, first_position, counter, 0);
    }

add_new_items:

  if (n_added > 0)
    {
      GSequenceIter *iter = g_sequence_get_iter_at_pos (self->child_seq, position);
      gint64 previous_sort_position = -1;
      gint64 first_position = -1;
      gint64 counter = 0;

      for (i = 0; i < n_added; i++)
        {
          g_autoptr (GObject) instance = NULL;
          GSequenceIter *sort_iter;
          guint new_sort_position;

          instance = g_list_model_get_item (child_model, position + i);
          g_assert (G_IS_OBJECT (instance));

          sort_iter = g_sequence_insert_sorted (self->sort_seq,
                                                g_steal_pointer (&instance),
                                                seq_compare_func,
                                                self);

          new_sort_position = g_sequence_iter_get_position (sort_iter);

          /* Insert next item before this */
          iter = g_sequence_insert_before (iter, sort_iter);
          iter = g_sequence_iter_next (iter);

          if (first_position == -1)
            first_position = new_sort_position;

          /*
           * This happens when the position in the sorted sequence is different
           * from the position in the child sequence. We try to accumulate as many
           * items-changed() signals as possible, but we can't do that when the
           * order doesn't match.
           */
          if (previous_sort_position != -1 && new_sort_position != previous_sort_position + 1)
            {
              emit_items_changed (self, first_position, 0, counter);

              first_position = new_sort_position;
              counter = 0;
            }

          previous_sort_position = new_sort_position;
          counter++;
        }

      /*
       * Last items-changed() - if the child model is already sorted,
       * only this one will be fired.
       */
      if (counter > 0)
        emit_items_changed (self, first_position, 0, counter);
    }

  g_assert ((guint)g_sequence_get_length (self->child_seq) == g_list_model_get_n_items (child_model));
  g_assert ((guint)g_sequence_get_length (self->sort_seq) == g_list_model_get_n_items (child_model));

  GTD_EXIT;
}


/*
 * GListModel iface
 */

static GType
gtd_list_model_sort_get_item_type (GListModel *model)
{
  GtdListModelSort *self = (GtdListModelSort*) model;

  g_assert (GTD_IS_LIST_MODEL_SORT (self));

  return g_list_model_get_item_type (self->child_model);
}

static guint
gtd_list_model_sort_get_n_items (GListModel *model)
{
  GtdListModelSort *self = (GtdListModelSort*) model;

  g_assert (GTD_IS_LIST_MODEL_SORT (self));
  g_assert (self->sort_seq != NULL);

  return self->length;
}

static gpointer
gtd_list_model_sort_get_item (GListModel *model,
                              guint       position)
{
  GtdListModelSort *self;
  GSequenceIter *iter;
  gpointer item;

  g_assert (GTD_IS_LIST_MODEL_SORT (model));

  self = (GtdListModelSort*) model;
  iter = NULL;

  if (self->last_position != -1)
    {
      if (self->last_position == position + 1)
        iter = g_sequence_iter_prev (self->last_iter);
      else if (self->last_position == position - 1)
        iter = g_sequence_iter_next (self->last_iter);
      else if (self->last_position == position)
        iter = self->last_iter;
    }

  if (!iter)
    iter = g_sequence_get_iter_at_pos (self->sort_seq, position);

  if (g_sequence_iter_is_end (iter))
    return NULL;

  self->last_iter = iter;
  self->last_position = position;

  item = g_sequence_get (iter);
  g_assert (item != NULL);

  return g_object_ref (G_OBJECT (item));
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_item_type = gtd_list_model_sort_get_item_type;
  iface->get_n_items = gtd_list_model_sort_get_n_items;
  iface->get_item = gtd_list_model_sort_get_item;
}


/*
 * GObject overrides
 */

static void
gtd_list_model_sort_finalize (GObject *object)
{
  GtdListModelSort *self = (GtdListModelSort*) object;

  g_clear_pointer (&self->child_seq, g_sequence_free);
  g_clear_pointer (&self->sort_seq, g_sequence_free);

  if (self->compare_func_data_destroy)
    {
      g_clear_pointer (&self->compare_func_data, self->compare_func_data_destroy);
      self->compare_func_data_destroy = NULL;
    }

  g_clear_object (&self->child_model);

  G_OBJECT_CLASS (gtd_list_model_sort_parent_class)->finalize (object);
}

static void
gtd_list_model_sort_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GtdListModelSort *self = GTD_LIST_MODEL_SORT (object);

  switch (prop_id)
    {
    case PROP_CHILD_MODEL:
      g_value_set_object (value, gtd_list_model_sort_get_child_model (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_list_model_sort_class_init (GtdListModelSortClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_list_model_sort_finalize;
  object_class->get_property = gtd_list_model_sort_get_property;

  properties [PROP_CHILD_MODEL] = g_param_spec_object ("child-model",
                                                       "Child Model",
                                                       "The child model being sorted.",
                                                       G_TYPE_LIST_MODEL,
                                                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gtd_list_model_sort_init (GtdListModelSort *self)
{
  self->compare_func = default_compare_func;
  self->child_seq = g_sequence_new (gtd_list_model_sort_item_free);
  self->sort_seq = g_sequence_new (g_object_unref);
  self->last_position = -1;
}

GtdListModelSort *
gtd_list_model_sort_new (GListModel *child_model)
{
  GtdListModelSort *self;

  g_return_val_if_fail (G_IS_LIST_MODEL (child_model), NULL);

  self = g_object_new (GTD_TYPE_LIST_MODEL_SORT, NULL);
  self->child_model = g_object_ref (child_model);

  g_signal_connect_object (child_model,
                           "items-changed",
                           G_CALLBACK (child_model_items_changed),
                           self,
                           G_CONNECT_SWAPPED);

  child_model_items_changed (self, 0, 0, g_list_model_get_n_items (child_model), child_model);

  return self;
}

/**
 * gtd_list_model_sort_get_child_model:
 * @self: A #GtdListModelSort
 *
 * Gets the child model that is being sorted.
 *
 * Returns: (transfer none): A #GListModel.
 */
GListModel *
gtd_list_model_sort_get_child_model (GtdListModelSort *self)
{
  g_return_val_if_fail (GTD_IS_LIST_MODEL_SORT (self), NULL);

  return self->child_model;
}

void
gtd_list_model_sort_invalidate (GtdListModelSort *self)
{
  guint n_items;

  g_return_if_fail (GTD_IS_LIST_MODEL_SORT (self));

  /* First determine how many items we need to synthesize as a removal */
  n_items = g_sequence_get_length (self->child_seq);

  g_assert (G_IS_LIST_MODEL (self->child_model));

  invalidate_cache (self);

  g_sequence_sort (self->sort_seq, seq_compare_func, self);

  g_assert ((guint)g_sequence_get_length (self->child_seq) == n_items);
  g_assert ((guint)g_sequence_get_length (self->sort_seq) == n_items);

  if (n_items > 0)
    emit_items_changed (self, 0, n_items, n_items);
}

void
gtd_list_model_sort_set_sort_func (GtdListModelSort        *self,
                                   GtdListModelCompareFunc  compare_func,
                                   gpointer                 compare_func_data,
                                   GDestroyNotify           compare_func_data_destroy)
{
  g_return_if_fail (GTD_IS_LIST_MODEL_SORT (self));
  g_return_if_fail (compare_func || (!compare_func_data && !compare_func_data_destroy));

  GTD_ENTRY;

  if (self->compare_func_data_destroy != NULL)
    g_clear_pointer (&self->compare_func_data, self->compare_func_data_destroy);

  if (compare_func != NULL)
    {
      self->compare_func = compare_func;
      self->compare_func_data = compare_func_data;
      self->compare_func_data_destroy = compare_func_data_destroy;
    }
  else
    {
      self->compare_func = default_compare_func;
      self->compare_func_data = NULL;
      self->compare_func_data_destroy = NULL;
    }

  gtd_list_model_sort_invalidate (self);

  GTD_EXIT;
}
