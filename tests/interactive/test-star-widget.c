/* test-star-widget.c
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


#include <gtk/gtk.h>

#include "logging/gtd-log.h"
#include "widgets/gtd-star-widget.h"

gint
main (gint   argc,
      gchar *argv[])
{
  GtkWidget *start_widget = NULL;
  GtkWindow *window = NULL;

  g_set_prgname ("test-star-widget");
  g_set_application_name ("GNOME To Do | Star Widget");

  gtk_init ();
  gtd_log_init ();

  /* Box */
  start_widget = gtd_star_widget_new ();

  /* Window */
  window = GTK_WINDOW (gtk_window_new ());
  gtk_window_set_default_size (window, 200, 150);
  gtk_container_add (GTK_CONTAINER (window), start_widget);
  gtk_window_present (window);

  while (TRUE)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
