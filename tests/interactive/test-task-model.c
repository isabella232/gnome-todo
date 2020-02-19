/* test-task-model.c
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

#include <gtk/gtk.h>

#include "models/gtd-task-model.h"
#include "models/gtd-task-model-private.h"
#include "logging/gtd-log.h"
#include "dummy-provider.h"
#include "gtd-manager.h"
#include "gtd-manager-protected.h"
#include "gtd-provider.h"
#include "gtd-task.h"
#include "gtd-task-list.h"

static GtkWidget*
create_bold_label_for_task_row (GtkListBoxRow *row)
{
  g_autofree gchar *markup = NULL;
  GtdTask *task;

  task = g_object_get_data (G_OBJECT (row), "task");
  markup = g_strdup_printf ("<big><b>%s</b></big>", gtd_task_list_get_name (gtd_task_get_list (task)));

  return g_object_new (GTK_TYPE_LABEL,
                       "margin", 6,
                       "margin-top", 18,
                       "use-markup", TRUE,
                       "label", markup,
                       "xalign", 0.0,
                       NULL);
}

static void
header_func (GtkListBoxRow *row,
             GtkListBoxRow *before,
             gpointer       user_data)
{
  GtkWidget *header = NULL;

  if (!before)
    {
      header = create_bold_label_for_task_row (row);
    }
  else
    {
      GtdTask *before_task;
      GtdTask *task;

      before_task = g_object_get_data (G_OBJECT (before), "task");
      task = g_object_get_data (G_OBJECT (row), "task");

      if (gtd_task_get_list (task) != gtd_task_get_list (before_task))
        header = create_bold_label_for_task_row (row);
    }

  gtk_list_box_row_set_header (row, header);
}

static GtkWidget*
create_label_for_string (const gchar *string)
{
  return g_object_new (GTK_TYPE_LABEL,
                       "label", string,
                       "hexpand", TRUE,
                       "xalign", 0.0,
                       "margin", 6,
                       "margin-start", 18,
                       NULL);
}

static GtkWidget*
create_task_cb (gpointer item,
                gpointer user_data)
{
  GtkWidget *row;
  GtkWidget *box;
  GtdTask *task;

  task = item;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);

  gtk_container_add (GTK_CONTAINER (box), create_label_for_string (gtd_task_get_title (task)));

  row = gtk_list_box_row_new ();
  gtk_container_add (GTK_CONTAINER (row), box);

  g_object_set_data (G_OBJECT (row), "task", task);

  return row;
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr (DummyProvider) dummy_provider = NULL;
  GtdTaskModel *model = NULL;
  GtkWidget *scrolledwindow = NULL;
  GtkWindow *window = NULL;
  GtkWidget *listbox = NULL;

  g_set_prgname ("test-task-model");
  g_set_application_name ("GNOME To Do | Task Model Test");

  gtk_init ();
  gtd_log_init ();

  /* Create a DumbProvider and pre-populate it */
  dummy_provider = dummy_provider_new ();
  dummy_provider_generate_task_lists (dummy_provider);

  /* Inject a dumb fake provider */
  _gtd_manager_inject_provider (gtd_manager_get_default (), GTD_PROVIDER (dummy_provider));

  /* Now create the model - the initial providers must be there already */
  model = _gtd_task_model_new (gtd_manager_get_default ());

  /* Listbox */
  listbox = gtk_list_box_new ();
  gtk_list_box_bind_model (GTK_LIST_BOX (listbox),
                           G_LIST_MODEL (model),
                           create_task_cb,
                           NULL,
                           NULL);

  gtk_list_box_set_header_func (GTK_LIST_BOX (listbox), header_func, NULL, NULL);

  /* Scrolled window */
  scrolledwindow = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                                 "propagate-natural-height", TRUE,
                                 "max-content-height", 600,
                                 "hscrollbar-policy", GTK_POLICY_NEVER,
                                 NULL);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), listbox);

  /* Window */
  window = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window, 800, 600);
  gtk_container_add (GTK_CONTAINER (window), scrolledwindow);
  gtk_window_present (window);

  /* Now, generate more tasks and lists after injecting to the manager */
  dummy_provider_generate_task_lists (dummy_provider);

  /* Schedule a live removal of tasks */
  dummy_provider_schedule_remove_task (dummy_provider);

  while (TRUE)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
