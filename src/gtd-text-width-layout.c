/* gtd-text-width-layout.c
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

#include "gtd-text-width-layout.h"

struct _GtdTextWidthLayout
{
  GtkLayoutManager    parent;

  gint                max_width_chars;
  gint                width_chars;
};

G_DEFINE_TYPE (GtdTextWidthLayout, gtd_text_width_layout, GTK_TYPE_LAYOUT_MANAGER)

enum
{
  PROP_0,
  PROP_MAX_WIDTH_CHARS,
  PROP_WIDTH_CHARS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * GtkLayoutManager overrides
 */

static void
gtd_text_width_layout_measure (GtkLayoutManager *layout_manager,
                               GtkWidget        *widget,
                               GtkOrientation    orientation,
                               gint              for_size,
                               gint             *minimum,
                               gint             *natural,
                               gint             *minimum_baseline,
                               gint             *natural_baseline)
{
  GtdTextWidthLayout *self = (GtdTextWidthLayout *)layout_manager;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      gint child_min_baseline = -1;
      gint child_nat_baseline = -1;
      gint child_min = 0;
      gint child_nat = 0;

      if (!gtk_widget_should_layout (child))
        continue;

      gtk_widget_measure (child, orientation, for_size,
                          &child_min, &child_nat,
                          &child_min_baseline, &child_nat_baseline);

      *minimum = MAX (*minimum, child_min);
      *natural = MAX (*natural, child_nat);

      if (child_min_baseline > -1)
        *minimum_baseline = MAX (*minimum_baseline, child_min_baseline);
      if (child_nat_baseline > -1)
        *natural_baseline = MAX (*natural_baseline, child_nat_baseline);
    }

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      PangoFontMetrics *metrics;
      PangoContext *context;
      gint max_width_chars = G_MAXINT;
      gint width_chars = 0;
      gint char_width;
      gint digit_width;

      context = gtk_widget_get_pango_context (widget);
      metrics = pango_context_get_metrics (context,
                                           pango_context_get_font_description (context),
                                           pango_context_get_language (context));

      char_width = pango_font_metrics_get_approximate_char_width (metrics);
      digit_width = pango_font_metrics_get_approximate_digit_width (metrics);

      if (self->width_chars >= 0)
        width_chars = MAX (char_width, digit_width) * self->width_chars / PANGO_SCALE;

      if (self->max_width_chars >= 0)
        max_width_chars = MAX (char_width, digit_width) * self->max_width_chars / PANGO_SCALE;

      *minimum = MAX (*minimum, width_chars);
      *natural = MAX (*natural, max_width_chars);

      pango_font_metrics_unref (metrics);
    }
}

static void
gtd_text_width_layout_allocate (GtkLayoutManager *layout_manager,
                                GtkWidget        *widget,
                                gint              width,
                                gint              height,
                                gint              baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      if (child && gtk_widget_should_layout (child))
        gtk_widget_allocate (child, width, height, baseline, NULL);
    }
}


/*
 * GObject overrides
 */

static void
gtd_text_width_layout_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GtdTextWidthLayout *self = GTD_TEXT_WIDTH_LAYOUT (object);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_CHARS:
      g_value_set_int (value, self->max_width_chars);
      break;

    case PROP_WIDTH_CHARS:
      g_value_set_int (value, self->width_chars);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_text_width_layout_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GtdTextWidthLayout *self = GTD_TEXT_WIDTH_LAYOUT (object);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_CHARS:
      gtd_text_width_layout_set_max_width_chars (self, g_value_get_int (value));
      break;

    case PROP_WIDTH_CHARS:
      gtd_text_width_layout_set_width_chars (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_text_width_layout_class_init (GtdTextWidthLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  layout_manager_class->measure = gtd_text_width_layout_measure;
  layout_manager_class->allocate = gtd_text_width_layout_allocate;

  object_class->get_property = gtd_text_width_layout_get_property;
  object_class->set_property = gtd_text_width_layout_set_property;

  /**
   * GtdTextWidthLayout:max-width-chars:
   *
   * Sets the maximum size of the #GtkWidget in characters.
   */
  properties[PROP_MAX_WIDTH_CHARS] = g_param_spec_int ("max-width-chars",
                                                       "Max Width Chars",
                                                       "Max Width Chars",
                                                       -1,
                                                       G_MAXINT,
                                                       -1,
                                                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * GtdTextWidthLayout:width-chars:
   *
   * Sets the size of the #GtkWidget in characters.
   */
  properties[PROP_WIDTH_CHARS] = g_param_spec_int ("width-chars",
                                                   "Width Chars",
                                                   "Width Chars",
                                                   -1,
                                                   G_MAXINT,
                                                   -1,
                                                   G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gtd_text_width_layout_init (GtdTextWidthLayout *self)
{
  self->max_width_chars = -1;
  self->width_chars = -1;
}

/**
 * gtd_text_width_layout_new:
 *
 * Creates a new #GtdTextWidthLayout.
 *
 * Returns: (transfer full): a #GtdTextWidthLayout
 */
GtkLayoutManager*
gtd_text_width_layout_new (void)
{
  return g_object_new (GTD_TYPE_TEXT_WIDTH_LAYOUT, NULL);
}

/**
 * gtd_text_width_layout_get_max_width_chars:
 *
 * Retrieves the maximum width in characters of @self.
 *
 * Returns: maximum width in characters
 */
gint
gtd_text_width_layout_get_max_width_chars (GtdTextWidthLayout *self)
{
  g_return_val_if_fail (GTD_IS_TEXT_WIDTH_LAYOUT (self), -1);

  return self->max_width_chars;
}

/**
 * gtd_text_width_layout_set_max_width_chars:
 * @self: a #GtdTextWidthLayout
 * @with_chars: maximum width of the widget @self is attached to, in character length
 *
 * Sets the maximum width @self has, in characters length. It is a programming
 * error to set a value smaller than #GtdTextWidthLayout:width-layout.
 */
void
gtd_text_width_layout_set_max_width_chars (GtdTextWidthLayout *self,
                                           gint                max_width_chars)
{
  g_return_if_fail (GTD_IS_TEXT_WIDTH_LAYOUT (self));
  g_return_if_fail (max_width_chars >= -1);
  g_return_if_fail (self->width_chars == -1 || max_width_chars >= self->width_chars);

  if (self->max_width_chars == max_width_chars)
    return;

  self->max_width_chars = max_width_chars;
  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_WIDTH_CHARS]);
}

/**
 * gtd_text_width_layout_get_width_chars:
 *
 * Retrieves the minimum width in characters of @self.
 *
 * Returns: minimum width in characters
 */
gint
gtd_text_width_layout_get_width_chars (GtdTextWidthLayout *self)
{
  g_return_val_if_fail (GTD_IS_TEXT_WIDTH_LAYOUT (self), -1);

  return self->width_chars;
}

/**
 * gtd_text_width_layout_set_width_chars:
 * @self: a #GtdTextWidthLayout
 * @with_chars: minimum width of the widget @self is attached to, in character length
 *
 * Sets the minimum width @self has, in characters length. It is a programming
 * error to set a value bigger than #GtdTextWidthLayout:max-width-layout.
 */
void
gtd_text_width_layout_set_width_chars (GtdTextWidthLayout *self,
                                       gint                width_chars)
{
  g_return_if_fail (GTD_IS_TEXT_WIDTH_LAYOUT (self));
  g_return_if_fail (width_chars >= -1);
  g_return_if_fail (self->max_width_chars == -1 || width_chars <= self->max_width_chars);

  if (self->width_chars == width_chars)
    return;

  self->width_chars = width_chars;
  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_WIDTH_CHARS]);
}
