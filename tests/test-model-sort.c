/* test-model-sort.c
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

#include "models/gtd-list-model-sort.h"

#include <math.h>
#include <string.h>

#define TEST_TYPE_ITEM (test_item_get_type())

struct _TestItem
{
  GObject p;
  guint n;
};

G_DECLARE_FINAL_TYPE (TestItem, test_item, TEST, ITEM, GObject)
G_DEFINE_TYPE (TestItem, test_item, G_TYPE_OBJECT)

static void
test_item_class_init (TestItemClass *klass)
{
}

static void
test_item_init (TestItem *self)
{
}

static TestItem *
test_item_new (guint n)
{
  TestItem *item;

  item = g_object_new (TEST_TYPE_ITEM, NULL);
  item->n = n;

  return item;
}

static gint
sort_func1 (GObject  *a,
            GObject  *b,
            gpointer  user_data)
{
  return TEST_ITEM (a)->n - TEST_ITEM (b)->n;
}

static gint
sort_func2 (GObject  *a,
            GObject  *b,
            gpointer  user_data)
{
  return (TEST_ITEM (a)->n & 1) - (TEST_ITEM (b)->n & 1);
}

static void
test_basic (void)
{
  GtdListModelSort *sort;
  GListStore *model;
  guint i;

  model = g_list_store_new (TEST_TYPE_ITEM);
  g_assert (model);

  sort = gtd_list_model_sort_new (G_LIST_MODEL (model));
  g_assert (sort);

  /* Test requesting past boundary */
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (sort), 0));
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (sort), 1));

  for (i = 0; i < 1000; i++)
    {
      g_autoptr (TestItem) val = test_item_new (1000 - i);
      g_list_store_append (model, val);
    }

  /* Test requesting past boundary */
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (sort), 1000));

  /* Ascendent sorting */
  gtd_list_model_sort_set_sort_func (sort, sort_func1, NULL, NULL);

  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (sort)));

  for (i = 1; i < 1000; i++)
    {
      g_autoptr (TestItem) current = g_list_model_get_item (G_LIST_MODEL (sort), i);
      g_autoptr (TestItem) previous = g_list_model_get_item (G_LIST_MODEL (sort), i - 1);

      g_assert_cmpint (previous->n, <, current->n);
    }

  /* Odd/Even sorting */
  gtd_list_model_sort_set_sort_func (sort, sort_func2, NULL, NULL);

  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (sort)));

  for (i = 0; i < 1000; i++)
    {
      g_autoptr (TestItem) current = g_list_model_get_item (G_LIST_MODEL (sort), i);

      if (i < 500)
        g_assert_cmpint (current->n % 2, ==, 0);
      else
        g_assert_cmpint (current->n % 2, ==, 1);
    }

  g_clear_object (&model);
  g_clear_object (&sort);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/models/model-sort/basic", test_basic);

  return g_test_run ();
}
