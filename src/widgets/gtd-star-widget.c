/* gtd-star-widget.c
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

#include "gtd-star-widget.h"

struct _GtdStarWidget
{
  GtkWidget           parent;

  GtkWidget          *filled_star;
  GtkWidget          *empty_star;

  gboolean            active;
};

G_DEFINE_TYPE (GtdStarWidget, gtd_star_widget, GTK_TYPE_WIDGET)

enum
{
  PROP_0,
  PROP_ACTIVE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * Callbacks
 */

static void
on_star_widget_clicked_cb (GtkGestureClick *gesture,
                           gint             n_press,
                           gdouble          x,
                           gdouble          y,
                           GtdStarWidget   *self)
{
  gtd_star_widget_set_active (self, !self->active);
  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}


/*
 * GObject overrides
 */

static void
gtd_star_widget_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtdStarWidget *self = GTD_STAR_WIDGET (object);

  switch (prop_id)
    {
    case PROP_ACTIVE:
      g_value_set_boolean (value, self->active);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_star_widget_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtdStarWidget *self = GTD_STAR_WIDGET (object);

  switch (prop_id)
    {
    case PROP_ACTIVE:
      gtd_star_widget_set_active (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_star_widget_class_init (GtdStarWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = gtd_star_widget_get_property;
  object_class->set_property = gtd_star_widget_set_property;

  /**
   * GtdStarWidget:active:
   *
   * Whether the star widget is active or not. When active, the
   * star appears filled.
   */
  properties[PROP_ACTIVE] = g_param_spec_boolean ("active",
                                                  "Active",
                                                  "Active",
                                                  FALSE,
                                                  G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "star");
}

static void
gtd_star_widget_init (GtdStarWidget *self)
{
  GtkGesture *click_gesture;

  click_gesture = gtk_gesture_click_new ();
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (click_gesture), FALSE);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (click_gesture), TRUE);
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click_gesture), GDK_BUTTON_PRIMARY);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (click_gesture), GTK_PHASE_CAPTURE);
  g_signal_connect (click_gesture, "pressed", G_CALLBACK (on_star_widget_clicked_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (click_gesture));

  gtk_widget_set_cursor_from_name (GTK_WIDGET (self), "default");

  self->empty_star = gtk_image_new_from_icon_name ("non-starred-symbolic");
  gtk_widget_set_parent (self->empty_star, GTK_WIDGET (self));

  self->filled_star = gtk_image_new_from_icon_name ("starred-symbolic");
  gtk_widget_set_parent (self->filled_star, GTK_WIDGET (self));
  gtk_widget_hide (self->filled_star);

  g_object_bind_property (self->filled_star,
                          "visible",
                          self->empty_star,
                          "visible",
                          G_BINDING_INVERT_BOOLEAN | G_BINDING_SYNC_CREATE);
}

GtkWidget*
gtd_star_widget_new (void)
{
  return g_object_new (GTD_TYPE_STAR_WIDGET, NULL);
}

gboolean
gtd_star_widget_get_active (GtdStarWidget *self)
{
  g_return_val_if_fail (GTD_IS_STAR_WIDGET (self), FALSE);

  return self->active;
}

void
gtd_star_widget_set_active (GtdStarWidget *self,
                            gboolean       active)
{
  g_return_if_fail (GTD_IS_STAR_WIDGET (self));

  if (self->active == active)
    return;

  self->active = active;
  gtk_widget_set_visible (self->filled_star, active);

  if (active)
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED, FALSE);
  else
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED);

  /* TODO: explosion effect */

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
}
