/* gtd-widget.c
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

#include "gtd-debug.h"
#include "gtd-widget-private.h"

#include <graphene-gobject.h>

enum
{
  X,
  Y,
  Z,
};

typedef struct
{
  graphene_point3d_t  pivot_point;
  gfloat              rotation[3];
  gfloat              scale[3];
  graphene_point3d_t  translation;

  GtkAllocation       geometry;
  GskTransform       *cached_transform;
} GtdWidgetPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GtdWidget, gtd_widget, GTK_TYPE_WIDGET)

enum
{
  PROP_0,
  PROP_PIVOT_POINT,
  PROP_ROTATION_X,
  PROP_ROTATION_Y,
  PROP_ROTATION_Z,
  PROP_SCALE_X,
  PROP_SCALE_Y,
  PROP_SCALE_Z,
  PROP_TRANSLATION_X,
  PROP_TRANSLATION_Y,
  PROP_TRANSLATION_Z,
  N_PROPS
};

static GParamSpec *properties [N_PROPS] = { NULL, };


/*
 * Auxiliary methods
 */

static void
invalidate_cached_transform (GtdWidget *self)
{
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);

  g_clear_pointer (&priv->cached_transform, gsk_transform_unref);
}

static void
calculate_transform (GtdWidget *self)
{
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);
  graphene_point3d_t pivot;
  GskTransform *transform;
  gboolean pivot_is_zero;

  transform = NULL;

  /* Pivot point */
  pivot_is_zero = graphene_point3d_equal (&priv->pivot_point, graphene_point3d_zero ());
  pivot = GRAPHENE_POINT3D_INIT (priv->geometry.x + priv->geometry.width * priv->pivot_point.x,
                                 priv->geometry.y + priv->geometry.height * priv->pivot_point.y,
                                 priv->pivot_point.z);
  if (!pivot_is_zero)
    transform = gsk_transform_translate_3d (transform, &pivot);

  /* Perspective */
  transform = gsk_transform_perspective (transform,
                                         2 * MAX (priv->geometry.width, priv->geometry.height));

  /* Rotation */
  transform = gsk_transform_rotate_3d (transform, priv->rotation[X], graphene_vec3_x_axis ());
  transform = gsk_transform_rotate_3d (transform, priv->rotation[Y], graphene_vec3_y_axis ());
  transform = gsk_transform_rotate_3d (transform, priv->rotation[Z], graphene_vec3_z_axis ());

  /* Scale */
  if (G_APPROX_VALUE (priv->scale[Z], 1.f, FLT_EPSILON))
    transform = gsk_transform_scale (transform, priv->scale[X], priv->scale[Y]);
  else
    transform = gsk_transform_scale_3d (transform, priv->scale[X], priv->scale[Y], priv->scale[Z]);

  /* Translation */
  if (G_APPROX_VALUE (priv->translation.z, 0.f, FLT_EPSILON))
    {
      transform = gsk_transform_translate (transform,
                                           &GRAPHENE_POINT_INIT (priv->translation.x,
                                                                 priv->translation.y));
    }
  else
    {
      transform = gsk_transform_translate_3d (transform, &priv->translation);
    }

  /* Rollback pivot point */
  if (!pivot_is_zero)
    transform = gsk_transform_translate_3d (transform,
                                            &GRAPHENE_POINT3D_INIT (-pivot.x,
                                                                    -pivot.y,
                                                                    -pivot.z));

  priv->cached_transform = transform;
}


/*
 * GObject overrides
 */

static void
gtd_widget_dispose (GObject *object)
{
  GtdWidget *self = GTD_WIDGET (object);
  GtkWidget *child = gtk_widget_get_first_child (GTK_WIDGET (object));

  while (child)
    {
      GtkWidget *next = gtk_widget_get_next_sibling (child);

      gtk_widget_unparent (child);
      child = next;
    }

  invalidate_cached_transform (self);

  G_OBJECT_CLASS (gtd_widget_parent_class)->dispose (object);
}

static void
gtd_widget_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  GtdWidget *self = GTD_WIDGET (object);
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_PIVOT_POINT:
      g_value_set_boxed (value, &priv->pivot_point);
      break;

    case PROP_ROTATION_X:
      g_value_set_float (value, priv->rotation[X]);
      break;

    case PROP_ROTATION_Y:
      g_value_set_float (value, priv->rotation[Y]);
      break;

    case PROP_ROTATION_Z:
      g_value_set_float (value, priv->rotation[Z]);
      break;

    case PROP_SCALE_X:
      g_value_set_float (value, priv->scale[X]);
      break;

    case PROP_SCALE_Y:
      g_value_set_float (value, priv->scale[Y]);
      break;

    case PROP_SCALE_Z:
      g_value_set_float (value, priv->scale[Z]);
      break;

    case PROP_TRANSLATION_X:
      g_value_set_float (value, priv->translation.x);
      break;

    case PROP_TRANSLATION_Y:
      g_value_set_float (value, priv->translation.y);
      break;

    case PROP_TRANSLATION_Z:
      g_value_set_float (value, priv->translation.z);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_widget_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  GtdWidget *self = GTD_WIDGET (object);
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_PIVOT_POINT:
      gtd_widget_set_pivot_point (self, g_value_get_boxed (value));
      break;

    case PROP_ROTATION_X:
      gtd_widget_set_rotation (self, g_value_get_float (value), priv->rotation[Y], priv->rotation[Z]);
      break;

    case PROP_ROTATION_Y:
      gtd_widget_set_rotation (self, priv->rotation[X], g_value_get_float (value), priv->rotation[Z]);
      break;

    case PROP_ROTATION_Z:
      gtd_widget_set_rotation (self, priv->rotation[X], priv->rotation[Y], g_value_get_float (value));
      break;

    case PROP_SCALE_X:
      gtd_widget_set_scale (self, g_value_get_float (value), priv->scale[Y], priv->scale[Z]);
      break;

    case PROP_SCALE_Y:
      gtd_widget_set_scale (self, priv->scale[X], g_value_get_float (value), priv->scale[Z]);
      break;

    case PROP_SCALE_Z:
      gtd_widget_set_scale (self, priv->scale[X], priv->scale[Y], g_value_get_float (value));
      break;

    case PROP_TRANSLATION_X:
      gtd_widget_set_translation (self, g_value_get_float (value), priv->translation.y, priv->translation.z);
      break;

    case PROP_TRANSLATION_Y:
      gtd_widget_set_translation (self, priv->translation.x, g_value_get_float (value), priv->translation.z);
      break;

    case PROP_TRANSLATION_Z:
      gtd_widget_set_translation (self, priv->translation.x, priv->translation.y, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_widget_class_init (GtdWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = gtd_widget_dispose;
  object_class->get_property = gtd_widget_get_property;
  object_class->set_property = gtd_widget_set_property;

  /**
   * GtdWidget:
   */
  properties[PROP_PIVOT_POINT] = g_param_spec_boxed ("pivot-point",
                                                     "Pivot point",
                                                     "Pivot point",
                                                     GRAPHENE_TYPE_POINT3D,
                                                     G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:rotation-x
   */
  properties[PROP_ROTATION_X] = g_param_spec_float ("rotation-x",
                                                    "Rotation in the X axis",
                                                    "Rotation in the X axis",
                                                    -G_MAXFLOAT,
                                                    G_MAXFLOAT,
                                                    0.f,
                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:rotation-y
   */
  properties[PROP_ROTATION_Y] = g_param_spec_float ("rotation-y",
                                                    "Rotation in the Y axis",
                                                    "Rotation in the Y axis",
                                                    -G_MAXFLOAT,
                                                    G_MAXFLOAT,
                                                    0.f,
                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:rotation-z
   */
  properties[PROP_ROTATION_Z] = g_param_spec_float ("rotation-z",
                                                    "Rotation in the Z axis",
                                                    "Rotation in the Z axis",
                                                    -G_MAXFLOAT,
                                                    G_MAXFLOAT,
                                                    0.f,
                                                    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:scale-x
   */
  properties[PROP_SCALE_X] = g_param_spec_float ("scale-x",
                                                 "Scale in the X axis",
                                                 "Scale in the X axis",
                                                 -G_MAXFLOAT,
                                                 G_MAXFLOAT,
                                                 1.f,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:scale-y
   */
  properties[PROP_SCALE_Y] = g_param_spec_float ("scale-y",
                                                 "Scale in the Y axis",
                                                 "Scale in the Y axis",
                                                 -G_MAXFLOAT,
                                                 G_MAXFLOAT,
                                                 1.f,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:scale-z
   */
  properties[PROP_SCALE_Z] = g_param_spec_float ("scale-z",
                                                 "Scale in the Z axis",
                                                 "Scale in the Z axis",
                                                 -G_MAXFLOAT,
                                                 G_MAXFLOAT,
                                                 1.f,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:translation-x
   */
  properties[PROP_TRANSLATION_X] = g_param_spec_float ("translation-x",
                                                       "Translation in the X axis",
                                                       "Translation in the X axis",
                                                       -G_MAXFLOAT,
                                                       G_MAXFLOAT,
                                                       0.f,
                                                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:translation-y
   */
  properties[PROP_TRANSLATION_Y] = g_param_spec_float ("translation-y",
                                                       "Translation in the Y axis",
                                                       "Translation in the Y axis",
                                                       -G_MAXFLOAT,
                                                       G_MAXFLOAT,
                                                       0.f,
                                                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdWidget:translation-z
   */
  properties[PROP_TRANSLATION_Z] = g_param_spec_float ("translation-z",
                                                       "Translation in the Z axis",
                                                       "Translation in the Z axis",
                                                       -G_MAXFLOAT,
                                                       G_MAXFLOAT,
                                                       0.f,
                                                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
}

static void
gtd_widget_init (GtdWidget *self)
{
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);

  priv->scale[X] = 1.f;
  priv->scale[Y] = 1.f;
  priv->scale[Z] = 1.f;

  priv->pivot_point = GRAPHENE_POINT3D_INIT (0.5, 0.5, 0.f);
}

GtkWidget*
gtd_widget_new (void)
{
  return g_object_new (GTD_TYPE_WIDGET, NULL);
}

/**
 */
void
gtd_widget_get_pivot_point (GtdWidget          *self,
                            graphene_point3d_t *out_pivot_point)
{
  GtdWidgetPrivate *priv;

  g_return_if_fail (GTD_IS_WIDGET (self));
  g_return_if_fail (out_pivot_point != NULL);

  priv = gtd_widget_get_instance_private (self);
  *out_pivot_point = priv->pivot_point;
}

/**
 */
void
gtd_widget_set_pivot_point (GtdWidget                *self,
                            const graphene_point3d_t *pivot_point)
{
  GtdWidgetPrivate *priv;

  GTD_ENTRY;

  g_return_if_fail (GTD_IS_WIDGET (self));
  g_return_if_fail (pivot_point != NULL);
  g_return_if_fail (pivot_point->x >= 0.f && pivot_point->x <= 1.0);
  g_return_if_fail (pivot_point->y >= 0.f && pivot_point->y <= 1.0);

  priv = gtd_widget_get_instance_private (self);

  if (graphene_point3d_equal (&priv->pivot_point, pivot_point))
    GTD_RETURN ();

  invalidate_cached_transform (self);
  priv->pivot_point = *pivot_point;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  GTD_EXIT;
}

/**
 */
void
gtd_widget_get_rotation (GtdWidget *self,
                         gfloat    *rotation_x,
                         gfloat    *rotation_y,
                         gfloat    *rotation_z)
{
  GtdWidgetPrivate *priv;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);

  if (rotation_x)
    *rotation_x = priv->rotation[X];

  if (rotation_y)
    *rotation_y = priv->rotation[Y];

  if (rotation_z)
    *rotation_z = priv->rotation[Z];
}

/**
 */
void
gtd_widget_set_rotation (GtdWidget *self,
                         gfloat     rotation_x,
                         gfloat     rotation_y,
                         gfloat     rotation_z)
{
  GtdWidgetPrivate *priv;
  gboolean changed[3];

  GTD_ENTRY;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);

  changed[X] = !G_APPROX_VALUE (priv->rotation[X], rotation_x, FLT_EPSILON);
  changed[Y] = !G_APPROX_VALUE (priv->rotation[Y], rotation_y, FLT_EPSILON);
  changed[Z] = !G_APPROX_VALUE (priv->rotation[Z], rotation_z, FLT_EPSILON);

  if (!changed[X] && !changed[Y] && !changed[Z])
    GTD_RETURN ();

  invalidate_cached_transform (self);

  priv->rotation[X] = rotation_x;
  priv->rotation[Y] = rotation_y;
  priv->rotation[Z] = rotation_z;

  if (changed[X])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROTATION_X]);

  if (changed[Y])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROTATION_Y]);

  if (changed[Z])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROTATION_Z]);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  GTD_EXIT;
}

/**
 */
void
gtd_widget_get_scale (GtdWidget *self,
                      gfloat    *scale_x,
                      gfloat    *scale_y,
                      gfloat    *scale_z)
{
  GtdWidgetPrivate *priv;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);

  if (scale_x)
    *scale_x = priv->scale[0];

  if (scale_y)
    *scale_y = priv->scale[1];

  if (scale_z)
    *scale_z = priv->scale[2];
}

/**
 */
void
gtd_widget_set_scale (GtdWidget *self,
                      gfloat     scale_x,
                      gfloat     scale_y,
                      gfloat     scale_z)
{
  GtdWidgetPrivate *priv;
  gboolean changed[3];

  GTD_ENTRY;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);

  changed[X] = !G_APPROX_VALUE (priv->scale[X], scale_x, FLT_EPSILON);
  changed[Y] = !G_APPROX_VALUE (priv->scale[Y], scale_y, FLT_EPSILON);
  changed[Z] = !G_APPROX_VALUE (priv->scale[Z], scale_z, FLT_EPSILON);

  if (!changed[X] && !changed[Y] && !changed[Z])
    GTD_RETURN ();

  invalidate_cached_transform (self);

  priv->scale[X] = scale_x;
  priv->scale[Y] = scale_y;
  priv->scale[Z] = scale_z;

  if (changed[X])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCALE_X]);

  if (changed[Y])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCALE_Y]);

  if (changed[Z])
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCALE_Z]);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  GTD_EXIT;
}

/**
 */
void
gtd_widget_get_translation (GtdWidget *self,
                            gfloat    *translation_x,
                            gfloat    *translation_y,
                            gfloat    *translation_z)
{
  GtdWidgetPrivate *priv;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);

  if (translation_x)
    *translation_x = priv->translation.x;

  if (translation_y)
    *translation_y = priv->translation.y;

  if (translation_z)
    *translation_z = priv->translation.z;
}

/**
 */
void
gtd_widget_set_translation (GtdWidget *self,
                            gfloat     translation_x,
                            gfloat     translation_y,
                            gfloat     translation_z)
{
  graphene_point3d_t old_translation, translation;
  GtdWidgetPrivate *priv;

  GTD_ENTRY;

  g_return_if_fail (GTD_IS_WIDGET (self));

  priv = gtd_widget_get_instance_private (self);
  translation = GRAPHENE_POINT3D_INIT (translation_x, translation_y, translation_z);

  if (graphene_point3d_equal (&priv->translation, &translation))
    GTD_RETURN ();

  old_translation = priv->translation;

  invalidate_cached_transform (self);
  priv->translation = translation;

  if (!G_APPROX_VALUE (old_translation.x, translation.x, FLT_EPSILON))
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRANSLATION_X]);

  if (!G_APPROX_VALUE (old_translation.y, translation.y, FLT_EPSILON))
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRANSLATION_Y]);

  if (!G_APPROX_VALUE (old_translation.y, translation.y, FLT_EPSILON))
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TRANSLATION_Z]);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  GTD_EXIT;
}

/**
 */
GskTransform*
gtd_widget_apply_transform (GtdWidget    *self,
                            GskTransform *transform)
{
  GtdWidgetPrivate *priv;

  g_return_val_if_fail (GTD_IS_WIDGET (self), NULL);

  priv = gtd_widget_get_instance_private (self);

  if (!priv->cached_transform)
    calculate_transform (self);

  if (!transform)
    return gsk_transform_ref (priv->cached_transform);

  return gsk_transform_transform (transform, priv->cached_transform);
}

void
gtd_widget_update_pivot_for_geometry (GtdWidget           *self,
                                      const GtkAllocation *geometry)
{
  GtdWidgetPrivate *priv = gtd_widget_get_instance_private (self);

  if (priv->geometry.x != geometry->x ||
      priv->geometry.y != geometry->y ||
      priv->geometry.width != geometry->width ||
      priv->geometry.height != geometry->height)
    {
      invalidate_cached_transform (self);
      priv->geometry = *geometry;
    }
}
