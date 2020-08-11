/* test-widget.c
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

#include "gtd-widget.h"

static const char *css =
"translated {"
"  background-image: none;"
"  background-color: red;"
"}\n"
"scaled {"
"  background-image: none;"
"  background-color: green;"
"}\n"
"rotated {"
"  background-image: none;"
"  background-color: blue;"
"}\n"
;

static const char *ui =
"<interface>"
"  <object class='GtkWindow' id='window'>"
"    <property name='default-width'>400</property>"
"    <property name='default-height'>300</property>"
"    <child>"
"      <object class='GtdWidget'>"
"        <child>"
"          <object class='GtdWidget' id='translated'>"
"            <property name='css-name'>translated</property>"
"            <property name='translation-x'>30.0</property>"
"            <property name='translation-y'>15.0</property>"
"            <property name='translation-z'>0.0</property>"
"            <property name='valign'>start</property>"
"            <property name='width-request'>30</property>"
"            <property name='height-request'>30</property>"
"          </object>"
"        </child>"
"        <child>"
"          <object class='GtdWidget' id='scaled'>"
"            <property name='css-name'>scaled</property>"
"            <property name='scale-x'>0.75</property>"
"            <property name='scale-y'>1.25</property>"
"            <property name='scale-z'>1.0</property>"
"            <property name='valign'>center</property>"
"            <property name='width-request'>30</property>"
"            <property name='height-request'>30</property>"
"          </object>"
"        </child>"
"        <child>"
"          <object class='GtdWidget' id='rotated'>"
"            <property name='css-name'>rotated</property>"
"            <property name='rotation-x'>30.0</property>"
"            <property name='rotation-y'>-15.0</property>"
"            <property name='rotation-z'>0</property>"
"            <property name='valign'>end</property>"
"            <property name='width-request'>30</property>"
"            <property name='height-request'>30</property>"
"          </object>"
"        </child>"
"      </object>"
"    </child>"
"  </object>"
"</interface>";

static GtkWidget *
create_ui (void)
{
  g_autoptr (GtkBuilder) builder = NULL;
  g_autoptr (GError) error = NULL;
  GtkWidget *win;

  g_type_ensure (GTD_TYPE_WIDGET);

  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_string (builder, ui, -1, &error))
    {
      g_warning ("%s", error->message);
      return NULL;
    }

  win = (GtkWidget *)gtk_builder_get_object (builder, "window");
  g_object_ref (win);

  return win;
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr (GtkCssProvider) css_provider = NULL;
  GtkWindow *window;

  g_set_prgname ("test-colorbutton");
  g_set_application_name ("GNOME To Do | Widget Test");

  gtk_init ();

  css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (css_provider, css, -1);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window = GTK_WINDOW (create_ui ());
  gtk_window_present (window);

  while (TRUE)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
