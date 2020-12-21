/* gtd-menu-button.c
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

#include "gtd-menu-button.h"

typedef struct
{
  GIcon              *icon;

  GtkWidget          *button;
  GtkWidget          *popover; /* Only one at a time can be set */
  GMenuModel         *model;

  GtdMenuButtonCreatePopupFunc create_popup_func;
  gpointer            create_popup_user_data;
  GDestroyNotify      create_popup_destroy_notify;

  GtkWidget          *label_widget;
  GtkWidget          *align_widget;
  GtkWidget          *arrow_widget;
  GtkArrowType        arrow_type;
} GtdMenuButtonPrivate;

enum
{
  PROP_0,
  PROP_MENU_MODEL,
  PROP_ALIGN_WIDGET,
  PROP_DIRECTION,
  PROP_POPOVER,
  PROP_GICON,
  PROP_LABEL,
  PROP_USE_UNDERLINE,
  PROP_HAS_FRAME,
  LAST_PROP
};

static GParamSpec *menu_button_props[LAST_PROP];

G_DEFINE_TYPE_WITH_PRIVATE (GtdMenuButton, gtd_menu_button, GTK_TYPE_WIDGET)

static void
update_sensitivity (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  gtk_widget_set_sensitive (priv->button,
                            priv->popover != NULL ||
                            priv->create_popup_func != NULL);
}

static void
update_popover_direction (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  if (!priv->popover)
    return;

  switch (priv->arrow_type)
    {
    case GTK_ARROW_UP:
      gtk_popover_set_position (GTK_POPOVER (priv->popover), GTK_POS_TOP);
      break;
    case GTK_ARROW_DOWN:
    case GTK_ARROW_NONE:
      gtk_popover_set_position (GTK_POPOVER (priv->popover), GTK_POS_BOTTOM);
      break;
    case GTK_ARROW_LEFT:
      gtk_popover_set_position (GTK_POPOVER (priv->popover), GTK_POS_LEFT);
      break;
    case GTK_ARROW_RIGHT:
      gtk_popover_set_position (GTK_POPOVER (priv->popover), GTK_POS_RIGHT);
      break;
    default:
      break;
    }
}

static void
set_arrow_type (GtkImage     *image,
                GtkArrowType  arrow_type)
{
  switch (arrow_type)
    {
    case GTK_ARROW_NONE:
      gtk_image_set_from_icon_name (image, "open-menu-symbolic");
      break;
    case GTK_ARROW_DOWN:
      gtk_image_set_from_icon_name (image, "pan-down-symbolic");
      break;
    case GTK_ARROW_UP:
      gtk_image_set_from_icon_name (image, "pan-up-symbolic");
      break;
    case GTK_ARROW_LEFT:
      gtk_image_set_from_icon_name (image, "pan-start-symbolic");
      break;
    case GTK_ARROW_RIGHT:
      gtk_image_set_from_icon_name (image, "pan-end-symbolic");
      break;
    default:
      break;
    }
}

static void
add_arrow (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv;
  GtkWidget *arrow;

  priv = gtd_menu_button_get_instance_private (self);

  arrow = gtk_image_new ();
  set_arrow_type (GTK_IMAGE (arrow), priv->arrow_type);
  gtk_button_set_child (GTK_BUTTON (priv->button), arrow);
  priv->arrow_widget = arrow;
}

static void
set_align_widget_pointer (GtdMenuButton *self,
                          GtkWidget     *align_widget)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  if (priv->align_widget)
    g_object_remove_weak_pointer (G_OBJECT (priv->align_widget), (gpointer *) &priv->align_widget);

  priv->align_widget = align_widget;

  if (priv->align_widget)
    g_object_add_weak_pointer (G_OBJECT (priv->align_widget), (gpointer *) &priv->align_widget);
}


/*
 * Callbacks
 */

static gboolean
menu_deactivate_cb (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button), FALSE);

  return TRUE;
}

static void
popover_destroy_cb (GtdMenuButton *self)
{
  gtd_menu_button_set_popover (self, NULL);
}


/*
 * GtkWidget overrides
 */

static void
gtd_menu_button_state_flags_changed (GtkWidget    *widget,
                                     GtkStateFlags previous_state_flags)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (widget);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  if (!gtk_widget_is_sensitive (widget))
    {
      if (priv->popover)
        gtk_widget_hide (priv->popover);
    }
}

static void
gtd_menu_button_toggled (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  const gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->button));

  /* Might set a new menu/popover */
  if (active && priv->create_popup_func)
    {
      priv->create_popup_func (self, priv->create_popup_user_data);
    }

  if (priv->popover)
    {
      if (active)
        gtk_popover_popup (GTK_POPOVER (priv->popover));
      else
        gtk_popover_popdown (GTK_POPOVER (priv->popover));
    }
}

static void
gtd_menu_button_measure (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         int             for_size,
                         int            *minimum,
                         int            *natural,
                         int            *minimum_baseline,
                         int            *natural_baseline)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (widget);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  gtk_widget_measure (priv->button,
                      orientation,
                      for_size,
                      minimum, natural,
                      minimum_baseline, natural_baseline);

}

static void
gtd_menu_button_size_allocate (GtkWidget *widget,
                               int        width,
                               int        height,
                               int        baseline)
{
  GtdMenuButton *self= GTD_MENU_BUTTON (widget);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  gtk_widget_size_allocate (priv->button,
                            &(GtkAllocation) { 0, 0, width, height },
                            baseline);
  if (priv->popover)
    gtk_popover_present (GTK_POPOVER (priv->popover));
}

static gboolean
gtd_menu_button_focus (GtkWidget        *widget,
                       GtkDirectionType  direction)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (widget);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  if (priv->popover && gtk_widget_get_visible (priv->popover))
    return gtk_widget_child_focus (priv->popover, direction);
  else
    return gtk_widget_child_focus (priv->button, direction);
}

static gboolean
gtd_menu_button_grab_focus (GtkWidget *widget)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (widget);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  return gtk_widget_grab_focus (priv->button);
}


/*
 * GObject overrides
 */

static void
gtd_menu_button_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (object);

  switch (property_id)
    {
      case PROP_MENU_MODEL:
        gtd_menu_button_set_menu_model (self, g_value_get_object (value));
        break;

      case PROP_ALIGN_WIDGET:
        gtd_menu_button_set_align_widget (self, g_value_get_object (value));
        break;

      case PROP_DIRECTION:
        gtd_menu_button_set_direction (self, g_value_get_enum (value));
        break;

      case PROP_POPOVER:
        gtd_menu_button_set_popover (self, g_value_get_object (value));
        break;

      case PROP_GICON:
        gtd_menu_button_set_gicon (self, g_value_get_object (value));
        break;

      case PROP_LABEL:
        gtd_menu_button_set_label (self, g_value_get_string (value));
        break;

      case PROP_USE_UNDERLINE:
        gtd_menu_button_set_use_underline (self, g_value_get_boolean (value));
        break;

      case PROP_HAS_FRAME:
        gtd_menu_button_set_has_frame (self, g_value_get_boolean (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
gtd_menu_button_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (object);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  switch (property_id)
    {
      case PROP_MENU_MODEL:
        g_value_set_object (value, priv->model);
        break;

      case PROP_ALIGN_WIDGET:
        g_value_set_object (value, priv->align_widget);
        break;

      case PROP_DIRECTION:
        g_value_set_enum (value, priv->arrow_type);
        break;

      case PROP_POPOVER:
        g_value_set_object (value, priv->popover);
        break;

      case PROP_GICON:
        g_value_set_object (value, priv->icon);
        break;

      case PROP_LABEL:
        g_value_set_string (value, gtd_menu_button_get_label (GTD_MENU_BUTTON (object)));
        break;

      case PROP_USE_UNDERLINE:
        g_value_set_boolean (value, gtd_menu_button_get_use_underline (GTD_MENU_BUTTON (object)));
        break;

      case PROP_HAS_FRAME:
        g_value_set_boolean (value, gtd_menu_button_get_has_frame (GTD_MENU_BUTTON (object)));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
gtd_menu_button_dispose (GObject *object)
{
  GtdMenuButton *self = GTD_MENU_BUTTON (object);
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  if (priv->popover)
    {
      g_signal_handlers_disconnect_by_func (priv->popover, menu_deactivate_cb, object);
      g_signal_handlers_disconnect_by_func (priv->popover, popover_destroy_cb, object);
      gtk_widget_unparent (priv->popover);
      priv->popover = NULL;
    }

  set_align_widget_pointer (GTD_MENU_BUTTON (object), NULL);

  g_clear_object (&priv->model);
  g_clear_pointer (&priv->button, gtk_widget_unparent);

  if (priv->create_popup_destroy_notify)
    priv->create_popup_destroy_notify (priv->create_popup_user_data);

  G_OBJECT_CLASS (gtd_menu_button_parent_class)->dispose (object);
}

static void
gtd_menu_button_class_init (GtdMenuButtonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->set_property = gtd_menu_button_set_property;
  gobject_class->get_property = gtd_menu_button_get_property;
  gobject_class->dispose = gtd_menu_button_dispose;

  widget_class->measure = gtd_menu_button_measure;
  widget_class->size_allocate = gtd_menu_button_size_allocate;
  widget_class->state_flags_changed = gtd_menu_button_state_flags_changed;
  widget_class->focus = gtd_menu_button_focus;
  widget_class->grab_focus = gtd_menu_button_grab_focus;

  /**
   * GtdMenuButton:menu-model:
   *
   * The #GMenuModel from which the popup will be created.
   *
   * See gtd_menu_button_set_menu_model() for the interaction with the
   * #GtdMenuButton:popup property.
   */
  menu_button_props[PROP_MENU_MODEL] =
      g_param_spec_object ("menu-model",
                           "Menu model",
                           "The model from which the popup is made.",
                           G_TYPE_MENU_MODEL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdMenuButton:align-widget:
   *
   * The #GtkWidget to use to align the menu with.
   */
  menu_button_props[PROP_ALIGN_WIDGET] =
      g_param_spec_object ("align-widget",
                           "Align with",
                           "The parent widget which the menu should align with.",
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * GtdMenuButton:direction:
   *
   * The #GtkArrowType representing the direction in which the
   * menu or popover will be popped out.
   */
  menu_button_props[PROP_DIRECTION] =
      g_param_spec_enum ("direction",
                         "Direction",
                         "The direction the arrow should point.",
                         GTK_TYPE_ARROW_TYPE,
                         GTK_ARROW_DOWN,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * GtdMenuButton:popover:
   *
   * The #GtkPopover that will be popped up when the button is clicked.
   */
  menu_button_props[PROP_POPOVER] =
      g_param_spec_object ("popover",
                           "Popover",
                           "The popover",
                           GTK_TYPE_POPOVER,
                           G_PARAM_READWRITE);

  menu_button_props[PROP_GICON] =
      g_param_spec_string ("icon-name",
                           "Icon Name",
                           "The name of the icon used to automatically populate the button",
                           NULL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * GtdMenuButton:gicon:
   *
   */
  menu_button_props[PROP_GICON] =
      g_param_spec_object ("gicon",
                           "GIcon",
                           "A GIcon",
                           G_TYPE_ICON,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  menu_button_props[PROP_LABEL] =
      g_param_spec_string ("label",
                           "Label",
                           "The label for the button",
                           NULL,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  menu_button_props[PROP_USE_UNDERLINE] =
      g_param_spec_boolean ("use-underline",
                            "Use underline",
                            "If set, an underline in the text indicates the next character should be used for the mnemonic accelerator key",
                           FALSE,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  menu_button_props[PROP_HAS_FRAME] =
    g_param_spec_boolean ("has-frame",
                          "Has frame",
                          "Whether the button has a frame",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, LAST_PROP, menu_button_props);

  gtk_widget_class_set_css_name (widget_class, "menubutton");
}

static void
gtd_menu_button_init (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  priv->arrow_type = GTK_ARROW_DOWN;

  priv->button = gtk_toggle_button_new ();
  gtk_widget_set_parent (priv->button, GTK_WIDGET (self));
  g_signal_connect_swapped (priv->button, "toggled", G_CALLBACK (gtd_menu_button_toggled), self);
  add_arrow (self);

  gtk_widget_set_sensitive (priv->button, FALSE);

  gtk_widget_add_css_class (GTK_WIDGET (self), "popup");
}

/**
 * gtd_menu_button_new:
 *
 * Creates a new #GtdMenuButton widget with downwards-pointing
 * arrow as the only child. You can replace the child widget
 * with another #GtkWidget should you wish to.
 *
 * Returns: The newly created #GtdMenuButton widget
 */
GtkWidget *
gtd_menu_button_new (void)
{
  return g_object_new (GTD_TYPE_MENU_BUTTON, NULL);
}

/**
 * gtd_menu_button_set_menu_model:
 * @menu_button: a #GtdMenuButton
 * @menu_model: (nullable): a #GMenuModel, or %NULL to unset and disable the
 *   button
 *
 * Sets the #GMenuModel from which the popup will be constructed,
 * or %NULL to dissociate any existing menu model and disable the button.
 *
 * A #GtkPopover will be created from the menu model with gtk_popover_menu_new_from_model().
 * Actions will be connected as documented for this function.
 *
 * If #GtdMenuButton:popover is already set, it will be dissociated from the @menu_button,
 * and the property is set to %NULL.
 */
void
gtd_menu_button_set_menu_model (GtdMenuButton *self,
                                GMenuModel    *menu_model)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));
  g_return_if_fail (G_IS_MENU_MODEL (menu_model) || menu_model == NULL);

  g_object_freeze_notify (G_OBJECT (self));

  if (menu_model)
    g_object_ref (menu_model);

  if (menu_model)
    {
      GtkWidget *popover;

      popover = gtk_popover_menu_new_from_model (menu_model);
      gtd_menu_button_set_popover (self, popover);
    }
  else
    {
      gtd_menu_button_set_popover (self, NULL);
    }

  priv->model = menu_model;
  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_MENU_MODEL]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * gtd_menu_button_get_menu_model:
 * @menu_button: a #GtdMenuButton
 *
 * Returns the #GMenuModel used to generate the popup.
 *
 * Returns: (nullable) (transfer none): a #GMenuModel or %NULL
 */
GMenuModel *
gtd_menu_button_get_menu_model (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), NULL);

  return priv->model;
}

/**
 * gtd_menu_button_set_align_widget:
 * @menu_button: a #GtdMenuButton
 * @align_widget: (allow-none): a #GtkWidget
 *
 * Sets the #GtkWidget to use to line the menu with when popped up.
 * Note that the @align_widget must contain the #GtdMenuButton itself.
 *
 * Setting it to %NULL means that the menu will be aligned with the
 * button itself.
 *
 * Note that this property is only used with menus currently,
 * and not for popovers.
 */
void
gtd_menu_button_set_align_widget (GtdMenuButton *self,
                                  GtkWidget     *align_widget)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));
  g_return_if_fail (align_widget == NULL || gtk_widget_is_ancestor (GTK_WIDGET (self), align_widget));

  if (priv->align_widget == align_widget)
    return;

  set_align_widget_pointer (self, align_widget);

  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_ALIGN_WIDGET]);
}

/**
 * gtd_menu_button_get_align_widget:
 * @menu_button: a #GtdMenuButton
 *
 * Returns the parent #GtkWidget to use to line up with menu.
 *
 * Returns: (nullable) (transfer none): a #GtkWidget value or %NULL
 */
GtkWidget *
gtd_menu_button_get_align_widget (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), NULL);

  return priv->align_widget;
}

/**
 * gtd_menu_button_set_direction:
 * @menu_button: a #GtdMenuButton
 * @direction: a #GtkArrowType
 *
 * Sets the direction in which the popup will be popped up, as
 * well as changing the arrow’s direction. The child will not
 * be changed to an arrow if it was customized.
 *
 * If the does not fit in the available space in the given direction,
 * GTK+ will its best to keep it inside the screen and fully visible.
 *
 * If you pass %GTK_ARROW_NONE for a @direction, the popup will behave
 * as if you passed %GTK_ARROW_DOWN (although you won’t see any arrows).
 */
void
gtd_menu_button_set_direction (GtdMenuButton *self,
                               GtkArrowType   direction)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);
  GtkWidget *child;

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  if (priv->arrow_type == direction)
    return;

  priv->arrow_type = direction;
  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_DIRECTION]);

  /* Is it custom content? We don't change that */
  child = gtk_button_get_child (GTK_BUTTON (priv->button));
  if (priv->arrow_widget != child)
    return;

  set_arrow_type (GTK_IMAGE (child), priv->arrow_type);
  update_popover_direction (self);
}

/**
 * gtd_menu_button_get_direction:
 * @menu_button: a #GtdMenuButton
 *
 * Returns the direction the popup will be pointing at when popped up.
 *
 * Returns: a #GtkArrowType value
 */
GtkArrowType
gtd_menu_button_get_direction (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), GTK_ARROW_DOWN);

  return priv->arrow_type;
}

/**
 * gtd_menu_button_set_popover:
 * @menu_button: a #GtdMenuButton
 * @popover: (nullable): a #GtkPopover, or %NULL to unset and disable the button
 *
 * Sets the #GtkPopover that will be popped up when the @menu_button is clicked,
 * or %NULL to dissociate any existing popover and disable the button.
 *
 * If #GtdMenuButton:menu-model is set, the menu model is dissociated from the
 * @menu_button, and the property is set to %NULL.
 */
void
gtd_menu_button_set_popover (GtdMenuButton *self,
                             GtkWidget     *popover)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));
  g_return_if_fail (GTK_IS_POPOVER (popover) || popover == NULL);

  g_object_freeze_notify (G_OBJECT (self));

  g_clear_object (&priv->model);

  if (priv->popover)
    {
      if (gtk_widget_get_visible (priv->popover))
        gtk_widget_hide (priv->popover);

      g_signal_handlers_disconnect_by_func (priv->popover, menu_deactivate_cb, self);
      g_signal_handlers_disconnect_by_func (priv->popover, popover_destroy_cb, self);

      gtk_widget_unparent (priv->popover);
    }

  priv->popover = popover;

  if (popover)
    {
      gtk_widget_set_parent (priv->popover, GTK_WIDGET (self));
      g_signal_connect_swapped (priv->popover, "closed", G_CALLBACK (menu_deactivate_cb), self);
      g_signal_connect_swapped (priv->popover, "destroy", G_CALLBACK (popover_destroy_cb), self);
      update_popover_direction (self);
    }

  update_sensitivity (self);

  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_POPOVER]);
  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_MENU_MODEL]);
  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * gtd_menu_button_get_popover:
 * @menu_button: a #GtdMenuButton
 *
 * Returns the #GtkPopover that pops out of the button.
 * If the button is not using a #GtkPopover, this function
 * returns %NULL.
 *
 * Returns: (nullable) (transfer none): a #GtkPopover or %NULL
 */
GtkPopover *
gtd_menu_button_get_popover (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), NULL);

  return GTK_POPOVER (priv->popover);
}

/**
 * gtd_menu_button_set_icon_name:
 * @menu_button: a #GtdMenuButton
 * @icon_name: the icon name
 *
 * Sets the name of an icon to show inside the menu button.
 */
void
gtd_menu_button_set_gicon (GtdMenuButton *self,
                           GIcon         *icon)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);
  GtkWidget *box;
  GtkWidget *icon_image;
  GtkWidget *image;

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_margin_start (box, 6);
  gtk_widget_set_margin_end (box, 6);

  icon_image = gtk_image_new_from_gicon (icon);
  gtk_widget_set_hexpand (icon_image, TRUE);

  image = gtk_image_new_from_icon_name ("pan-down-symbolic");

  gtk_box_append (GTK_BOX (box), icon_image);
  gtk_box_append (GTK_BOX (box), image);
  gtk_button_set_child (GTK_BUTTON (priv->button), box);

  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_GICON]);
}

/**
 * gtd_menu_button_get_icon_name:
 * @menu_button: a #GtdMenuButton
 *
 * Gets the name of the icon shown in the button.
 *
 * Returns: the name of the icon shown in the button
 */
GIcon*
gtd_menu_button_get_gicon (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), NULL);

  return priv->icon;
}

/**
 * gtd_menu_button_set_label:
 * @menu_button: a #GtdMenuButton
 * @label: the label
 *
 * Sets the label to show inside the menu button.
 */
void
gtd_menu_button_set_label (GtdMenuButton *self,
                           const gchar   *label)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);
  GtkWidget *box;
  GtkWidget *label_widget;
  GtkWidget *image;

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_widget_set_margin_start (box, 6);
  gtk_widget_set_margin_end (box, 6);

  label_widget = gtk_label_new (label);
  gtk_label_set_xalign (GTK_LABEL (label_widget), 0);
  gtk_label_set_use_underline (GTK_LABEL (label_widget),
                               gtk_button_get_use_underline (GTK_BUTTON (priv->button)));
  gtk_widget_set_hexpand (label_widget, TRUE);
  image = gtk_image_new_from_icon_name ("pan-down-symbolic");
  gtk_box_append (GTK_BOX (box), label_widget);
  gtk_box_append (GTK_BOX (box), image);
  gtk_button_set_child (GTK_BUTTON (priv->button), box);
  priv->label_widget = label_widget;

  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_LABEL]);
}

/**
 * gtd_menu_button_get_label:
 * @menu_button: a #GtdMenuButton
 *
 * Gets the label shown in the button
 *
 * Returns: the label shown in the button
 */
const gchar*
gtd_menu_button_get_label (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);
  GtkWidget *child;

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), NULL);

  child = gtk_button_get_child (GTK_BUTTON (priv->button));
  if (GTK_IS_BOX (child))
    {
      child = gtk_widget_get_first_child (child);
      return gtk_label_get_label (GTK_LABEL (child));
    }

  return NULL;
}

/**
 * gtd_menu_button_set_has_frame:
 * @menu_button: a #GtdMenuButton
 * @has_frame: whether the button should have a visible frame
 *
 * Sets the style of the button.
 */
void
gtd_menu_button_set_has_frame (GtdMenuButton *self,
                               gboolean       has_frame)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  if (gtk_button_get_has_frame (GTK_BUTTON (priv->button)) == has_frame)
    return;

  gtk_button_set_has_frame (GTK_BUTTON (priv->button), has_frame);
  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_HAS_FRAME]);
}

/**
 * gtd_menu_button_get_has_frame:
 * @menu_button: a #GtdMenuButton
 *
 * Returns whether the button has a frame.
 *
 * Returns: %TRUE if the button has a frame
 */
gboolean
gtd_menu_button_get_has_frame (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), TRUE);

  return gtk_button_get_has_frame (GTK_BUTTON (priv->button));
}

/**
 * gtd_menu_button_popup:
 * @menu_button: a #GtdMenuButton
 *
 * Pop up the menu.
 */
void
gtd_menu_button_popup (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button), TRUE);
}

/**
 * gtd_menu_button_popdown:
 * @menu_button: a #GtdMenuButton
 *
 * Dismiss the menu.
 */
void
gtd_menu_button_popdown (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->button), FALSE);
}

/**
 * gtd_menu_button_set_create_popup_func:
 * @menu_button: a #GtdMenuButton
 * @func: (nullable): function to call when a popuop is about to
 *   be shown, but none has been provided via other means, or %NULL
 *   to reset to default behavior.
 * @user_data: (closure): user data to pass to @func.
 * @destroy_notify: (nullable): destroy notify for @user_data
 *
 * Sets @func to be called when a popup is about to be shown.
 * @func should use one of
 *
 *  - gtd_menu_button_set_popover()
 *  - gtd_menu_button_set_menu_model()
 *
 * to set a popup for @menu_button.
 * If @func is non-%NULL, @menu_button will always be sensitive.
 *
 * Using this function will not reset the menu widget attached to @menu_button.
 * Instead, this can be done manually in @func.
 */
void
gtd_menu_button_set_create_popup_func (GtdMenuButton                *self,
                                       GtdMenuButtonCreatePopupFunc  func,
                                       gpointer                      user_data,
                                       GDestroyNotify                destroy_notify)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  if (priv->create_popup_destroy_notify)
    priv->create_popup_destroy_notify (priv->create_popup_user_data);

  priv->create_popup_func = func;
  priv->create_popup_user_data = user_data;
  priv->create_popup_destroy_notify = destroy_notify;

  update_sensitivity (self);
}

/**
 * gtd_menu_button_set_use_underline:
 * @menu_button: a #GtdMenuButton
 * @use_underline: %TRUE if underlines in the text indicate mnemonics
 *
 * If true, an underline in the text indicates the next character should be
 * used for the mnemonic accelerator key.
 */
void
gtd_menu_button_set_use_underline (GtdMenuButton *self,
                                   gboolean       use_underline)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_if_fail (GTD_IS_MENU_BUTTON (self));

  if (gtk_button_get_use_underline (GTK_BUTTON (priv->button)) == use_underline)
    return;

  gtk_button_set_use_underline (GTK_BUTTON (priv->button), use_underline);
  if (priv->label_widget)
    gtk_label_set_use_underline (GTK_LABEL (priv->label_widget), use_underline);

  g_object_notify_by_pspec (G_OBJECT (self), menu_button_props[PROP_USE_UNDERLINE]);
}

/**
 * gtd_menu_button_get_use_underline:
 * @menu_button: a #GtdMenuButton
 *
 * Returns whether an embedded underline in the text indicates a
 * mnemonic. See gtd_menu_button_set_use_underline().
 *
 * Returns: %TRUE whether an embedded underline in the text indicates
 *     the mnemonic accelerator keys.
 */
gboolean
gtd_menu_button_get_use_underline (GtdMenuButton *self)
{
  GtdMenuButtonPrivate *priv = gtd_menu_button_get_instance_private (self);

  g_return_val_if_fail (GTD_IS_MENU_BUTTON (self), FALSE);

  return gtk_button_get_use_underline (GTK_BUTTON (priv->button));
}
