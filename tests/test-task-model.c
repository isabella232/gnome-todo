/* test-task-model.c
 *
 * Copyright 2018-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#include "gnome-todo.h"

#include "models/gtd-task-model.h"
#include "models/gtd-task-model-private.h"
#include "logging/gtd-log.h"
#include "gtd-manager-protected.h"
#include "dummy-provider.h"

static void
test_basic (void)
{
  g_autoptr (DummyProvider) dummy_provider = NULL;
  GtdTaskModel *model = NULL;
  guint n_tasks;

  /* Create a DumbProvider and pre-populate it */
  dummy_provider = dummy_provider_new ();
  n_tasks = dummy_provider_generate_task_lists (dummy_provider);
  gtd_manager_add_provider (gtd_manager_get_default (), GTD_PROVIDER (dummy_provider));

  model = _gtd_task_model_new (gtd_manager_get_default ());
  g_assert_cmpint (g_list_model_get_n_items (G_LIST_MODEL (model)), ==, n_tasks);

  /* Generate more */
  n_tasks = dummy_provider_generate_task_lists (dummy_provider);
  g_assert_cmpint (g_list_model_get_n_items (G_LIST_MODEL (model)), ==, n_tasks);

  while (n_tasks > 0)
    {
      n_tasks = dummy_provider_randomly_remove_task (dummy_provider);
      g_assert_cmpint (g_list_model_get_n_items (G_LIST_MODEL (model)), ==, n_tasks);
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);

  if (g_getenv ("G_MESSAGES_DEBUG"))
    gtd_log_init ();

  g_test_add_func ("/models/task-model/basic", test_basic);

  return g_test_run ();
}
