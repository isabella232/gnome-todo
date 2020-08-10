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

#include "gtd-keyframe-transition.h"
#include "gtd-widget.h"

static const char *css =
"translated {"
"  background-image: none;"
"  background-color: red;"
"}\n"
"wiggler {"
"  background-image: none;"
"  background-color: green;"
"}\n"
"rotated {"
"  background-image: none;"
"  background-color: blue;"
"}\n"
"mover {"
"  background-image: none;"
"  background-color: pink;"
"}\n"
;

static const char *ui =
"<interface>"
"  <object class='GtkWindow' id='window'>"
"    <property name='default-width'>600</property>"
"    <property name='default-height'>400</property>"
"    <child>"
"      <object class='GtkBox'>"
"        <child>"
"          <object class='GtdWidget'>"
"            <property name='hexpand'>true</property>"
"            <child>"
"              <object class='GtdWidget' id='translated'>"
"                <property name='css-name'>translated</property>"
"                <property name='halign'>center</property>"
"                <property name='valign'>center</property>"
"                <property name='width-request'>30</property>"
"                <property name='height-request'>30</property>"
"              </object>"
"            </child>"
"            <child>"
"              <object class='GtdWidget' id='wiggler'>"
"                <property name='css-name'>wiggler</property>"
"                <property name='translation-y'>40</property>"
"                <property name='halign'>center</property>"
"                <property name='valign'>start</property>"
"                <property name='width-request'>300</property>"
"                <property name='height-request'>40</property>"
"              </object>"
"            </child>"
"            <child>"
"              <object class='GtdWidget' id='rotated'>"
"                <property name='css-name'>rotated</property>"
"                <property name='translation-y'>-80</property>"
"                <property name='halign'>center</property>"
"                <property name='valign'>end</property>"
"                <property name='width-request'>40</property>"
"                <property name='height-request'>40</property>"
"              </object>"
"            </child>"
"            <child>"
"              <object class='GtdWidget' id='mover'>"
"                <property name='css-name'>mover</property>"
"                <property name='translation-x'>-200</property>"
"                <property name='translation-y'>-40</property>"
"                <property name='halign'>center</property>"
"                <property name='valign'>end</property>"
"                <property name='width-request'>50</property>"
"                <property name='height-request'>50</property>"
"              </object>"
"            </child>"
"          </object>"
"        </child>"
"        <child>"
"          <object class='GtkButton' id='button'>"
"            <property name='label'>Move</property>"
"            <property name='valign'>start</property>"
"            <property name='margin-top'>12</property>"
"            <property name='margin-start'>12</property>"
"            <property name='margin-end'>12</property>"
"            <property name='margin-bottom'>12</property>"
"          </object>"
"        </child>"
"      </object>"
"    </child>"
"  </object>"
"</interface>";

static gboolean pink_moved = FALSE;

static void
animate_rotation (GtdWidget *widget)
{
  GtdTransition *rotation_z;
  GtdTransition *scale_x;
  GtdTransition *scale_y;

  rotation_z = gtd_property_transition_new ("rotation-z");
  gtd_transition_set_from (rotation_z, G_TYPE_FLOAT, 0.f);
  gtd_transition_set_to (rotation_z, G_TYPE_FLOAT, 360.f);
  gtd_timeline_set_duration (GTD_TIMELINE (rotation_z), 750);
  gtd_timeline_set_repeat_count (GTD_TIMELINE (rotation_z), -1);
  gtd_timeline_set_auto_reverse (GTD_TIMELINE (rotation_z), TRUE);

  scale_x = gtd_property_transition_new ("scale-x");
  gtd_transition_set_from (scale_x, G_TYPE_FLOAT, 1.f);
  gtd_transition_set_to (scale_x, G_TYPE_FLOAT, 2.f);
  gtd_timeline_set_duration (GTD_TIMELINE (scale_x), 750);
  gtd_timeline_set_repeat_count (GTD_TIMELINE (scale_x), -1);
  gtd_timeline_set_auto_reverse (GTD_TIMELINE (scale_x), TRUE);

  scale_y = gtd_property_transition_new ("scale-y");
  gtd_transition_set_from (scale_y, G_TYPE_FLOAT, 1.f);
  gtd_transition_set_to (scale_y, G_TYPE_FLOAT, 2.f);
  gtd_timeline_set_duration (GTD_TIMELINE (scale_y), 750);
  gtd_timeline_set_repeat_count (GTD_TIMELINE (scale_y), -1);
  gtd_timeline_set_auto_reverse (GTD_TIMELINE (scale_y), TRUE);

  gtd_widget_add_transition (widget, "loop-rotation-z", rotation_z);
  gtd_widget_add_transition (widget, "loop-scale-x", scale_x);
  gtd_widget_add_transition (widget, "loop-scale-y", scale_y);
}

static void
animate_translation (GtdWidget *widget)
{
  GtdTransition *transition_x;

  transition_x = gtd_property_transition_new ("translation-x");
  gtd_transition_set_from (transition_x, G_TYPE_FLOAT, -200.f);
  gtd_transition_set_to (transition_x, G_TYPE_FLOAT, 200.f);
  gtd_timeline_set_duration (GTD_TIMELINE (transition_x), 2000);
  gtd_timeline_set_repeat_count (GTD_TIMELINE (transition_x), -1);
  gtd_timeline_set_auto_reverse (GTD_TIMELINE (transition_x), TRUE);

  gtd_widget_add_transition (widget, "loop-translation-x", transition_x);
}

static void
animate_wiggle (GtdWidget *widget)
{
  GtdTransition *transition_x;

  g_message ("Adding wiggle");

  gtd_widget_remove_all_transitions (widget);

  transition_x = gtd_keyframe_transition_new ("translation-x");
  gtd_transition_set_from (transition_x, G_TYPE_FLOAT, 0.f);
  gtd_transition_set_to (transition_x, G_TYPE_FLOAT, 0.f);
  gtd_timeline_set_duration (GTD_TIMELINE (transition_x), 350);
  gtd_timeline_set_delay (GTD_TIMELINE (transition_x), 1000);
  gtd_keyframe_transition_set (GTD_KEYFRAME_TRANSITION (transition_x),
                               G_TYPE_FLOAT,
                               5,
                               0.20, -15.f, GTD_EASE_OUT_QUAD,
                               0.40,  15.f, GTD_EASE_LINEAR,
                               0.60, -15.f, GTD_EASE_LINEAR,
                               0.80,  15.f, GTD_EASE_LINEAR,
                               1.00,   0.f, GTD_EASE_IN_QUAD);

  gtd_widget_add_transition (widget, "wiggle", transition_x);

  g_signal_connect_swapped (transition_x,
                            "completed",
                            G_CALLBACK (animate_wiggle),
                            widget);
}

static void
move_pink_cb (GtkButton *button,
              GtdWidget *widget)
{
  GtdTransition *rotation_y;

  gtd_widget_remove_all_transitions (widget);

  rotation_y = gtd_property_transition_new ("rotation-y");
  gtd_transition_set_from (rotation_y, G_TYPE_FLOAT, 0.f);
  gtd_transition_set_to (rotation_y, G_TYPE_FLOAT, 360.f);
  gtd_timeline_set_duration (GTD_TIMELINE (rotation_y), 500);
  gtd_timeline_set_repeat_count (GTD_TIMELINE (rotation_y), 3);

  gtd_widget_save_easing_state (widget);
  gtd_widget_set_easing_duration (widget, 2000);
  gtd_widget_set_easing_mode (widget, GTD_EASE_LINEAR);
  gtd_widget_set_translation (widget, pink_moved ? -200.f : 200.f, -40.f, 0.f);
  gtd_widget_restore_easing_state (widget);

  gtd_widget_add_transition (widget, "loop-rotation-y", rotation_y);

  pink_moved = !pink_moved;
}

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

  animate_rotation ((GtdWidget *)gtk_builder_get_object (builder, "rotated"));
  animate_translation ((GtdWidget *)gtk_builder_get_object (builder, "translated"));
  animate_wiggle ((GtdWidget *)gtk_builder_get_object (builder, "wiggler"));

  g_signal_connect (gtk_builder_get_object (builder, "button"),
                    "clicked",
                    G_CALLBACK (move_pink_cb),
                    gtk_builder_get_object (builder, "mover"));

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
