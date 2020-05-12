/* test-filter-sort.c
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


#include <gtk/gtk.h>

#include "models/gtd-list-model-filter.h"
#include "models/gtd-list-model-sort.h"
#include "models/gtd-task-model.h"
#include "models/gtd-task-model-private.h"
#include "logging/gtd-log.h"
#include "dummy-provider.h"
#include "gtd-manager.h"
#include "gtd-manager-protected.h"
#include "gtd-provider.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-utils.h"


/*
 * Auxiliary methods
 */

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


/*
 * Callbacks
 */

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

static gboolean
filter_func (GObject *item,
             gpointer user_data)
{
  g_autofree gchar *normalized_entry_text = NULL;
  g_autofree gchar *normalized_task_name = NULL;
  GtkEntry *entry;
  GtdTask *task;

  task = (GtdTask*) item;
  entry = (GtkEntry*) user_data;

  normalized_entry_text = gtd_normalize_casefold_and_unaccent (gtk_editable_get_text (GTK_EDITABLE (entry)));
  normalized_task_name = gtd_normalize_casefold_and_unaccent (gtd_task_get_title (task));

  return strstr (normalized_task_name, normalized_entry_text) != NULL;
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

  gtk_box_append (GTK_BOX (box), create_label_for_string (gtd_task_get_title (task)));

  row = gtk_list_box_row_new ();
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), box);

  g_object_set_data (G_OBJECT (row), "task", task);

  return row;
}

static gint
sort_func (GObject  *a,
           GObject  *b,
           gpointer  user_data)
{
  GtkToggleButton *check;
  GtdTask *task_a;
  GtdTask *task_b;

  check = (GtkToggleButton*) user_data;

  if (gtk_toggle_button_get_active (check))
    {
      task_a = GTD_TASK (a);
      task_b = GTD_TASK (b);
    }
  else
    {
      task_a = GTD_TASK (b);
      task_b = GTD_TASK (a);
    }

  if (gtd_task_get_list (task_a) == gtd_task_get_list (task_b))
    {
      return gtd_task_compare (task_b, task_a);
    }
  else
    {
      GtdTaskList *list_a = gtd_task_get_list (task_a);
      GtdTaskList *list_b = gtd_task_get_list (task_b);

      return g_strcmp0 (gtd_task_list_get_name (list_b), gtd_task_list_get_name (list_a));
    }
}

static void
on_check_active_changed_cb (GtkToggleButton  *check,
                            GParamSpec       *pspec,
                            GtdListModelSort *sort)
{
  gtd_list_model_sort_invalidate (sort);
}

static void
on_remove_button_clicked_cb (GtkButton     *button,
                             DummyProvider *provider)
{
  dummy_provider_randomly_remove_task (provider);
}

static void
on_search_text_changed_cb (GtkEntry           *entry,
                           GParamSpec         *pspec,
                           GtdListModelFilter *filter)
{
  gtd_list_model_filter_invalidate (filter);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr (GtdListModelFilter) filter = NULL;
  g_autoptr (GtdListModelSort) sort = NULL;
  g_autoptr (DummyProvider) dummy_provider = NULL;
  GtdTaskModel *model = NULL;
  GtkWidget *scrolledwindow = NULL;
  GtkWidget *search_entry = NULL;
  GtkWindow *window = NULL;
  GtkWidget *listbox = NULL;
  GtkWidget *button = NULL;
  GtkWidget *check = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;

  g_set_prgname ("test-filter-sort");
  g_set_application_name ("GNOME To Do | Filter & Sort Test");

  gtk_init ();
  gtd_log_init ();

  /* Create a DumbProvider and pre-populate it */
  dummy_provider = dummy_provider_new ();
  dummy_provider_generate_task_lists (dummy_provider);
  dummy_provider_generate_task_lists (dummy_provider);
  gtd_manager_add_provider (gtd_manager_get_default (), GTD_PROVIDER (dummy_provider));

  /* Models */
  model = _gtd_task_model_new (gtd_manager_get_default ());
  filter = gtd_list_model_filter_new (G_LIST_MODEL (model));
  sort = gtd_list_model_sort_new (G_LIST_MODEL (filter));

  /* Listbox */
  listbox = gtk_list_box_new ();
  gtk_list_box_bind_model (GTK_LIST_BOX (listbox),
                           G_LIST_MODEL (sort),
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
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolledwindow), listbox);

  /* Search entry */
  search_entry = gtk_search_entry_new ();
  gtd_list_model_filter_set_filter_func (filter, filter_func, search_entry, NULL);

  g_signal_connect_object (search_entry, "notify::text", G_CALLBACK (on_search_text_changed_cb), filter, 0);

  /* Reverse order checkbox */
  check = gtk_check_button_new_with_label ("Reverse Order");
  gtd_list_model_sort_set_sort_func (sort, sort_func, check, NULL);

  g_signal_connect_object (check, "notify::active", G_CALLBACK (on_check_active_changed_cb), sort, 0);

  /* Remove Tasks button */
  button = gtk_button_new_with_label ("Randomly Remove Tasks");
  g_signal_connect_object (button, "clicked", G_CALLBACK (on_remove_button_clicked_cb), dummy_provider, 0);

  /* Horizontal box */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (hbox), search_entry);
  gtk_box_append (GTK_BOX (hbox), check);
  gtk_box_append (GTK_BOX (hbox), button);

  /* Box */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_append (GTK_BOX (vbox), hbox);
  gtk_box_append (GTK_BOX (vbox), scrolledwindow);

  /* Window */
  window = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window, 800, 600);
  gtk_window_set_child (GTK_WINDOW (window), vbox);
  gtk_window_present (window);

  while (TRUE)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
