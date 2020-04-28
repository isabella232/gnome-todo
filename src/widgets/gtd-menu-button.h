/* gtd-menu-button.h
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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTD_TYPE_MENU_BUTTON (gtd_menu_button_get_type())
G_DECLARE_DERIVABLE_TYPE (GtdMenuButton, gtd_menu_button, GTD, MENU_BUTTON, GtkWidget)

struct _GtdMenuButtonClass
{
  GtkWidgetClass      parent_class;
};

/**
 * GtdMenuButtonCreatePopupFunc:
 * @menu_button: the #GtdMenuButton
 *
 * User-provided callback function to create a popup for @menu_button on demand.
 * This function is called when the popoup of @menu_button is shown, but none has
 * been provided via gtd_menu_buton_set_popup(), gtd_menu_button_set_popover()
 * or gtd_menu_button_set_menu_model().
 */
typedef void  (*GtdMenuButtonCreatePopupFunc) (GtdMenuButton *self,
                                               gpointer       user_data);

GtkWidget*           gtd_menu_button_new                         (void);

void                 gtd_menu_button_set_popover                 (GtdMenuButton      *self,
                                                                  GtkWidget          *popover);

GtkPopover*          gtd_menu_button_get_popover                 (GtdMenuButton      *self);

void                 gtd_menu_button_set_direction               (GtdMenuButton      *self,
                                                                  GtkArrowType        direction);

GtkArrowType         gtd_menu_button_get_direction               (GtdMenuButton      *self);

void                 gtd_menu_button_set_menu_model              (GtdMenuButton      *self,
                                                                  GMenuModel         *menu_model);

GMenuModel*          gtd_menu_button_get_menu_model              (GtdMenuButton      *self);


void                 gtd_menu_button_set_align_widget            (GtdMenuButton      *self,
                                                                  GtkWidget          *align_widget);

GtkWidget*           gtd_menu_button_get_align_widget            (GtdMenuButton      *self);


void                 gtd_menu_button_set_gicon                   (GtdMenuButton      *self,
                                                                  GIcon              *icon);

GIcon*               gtd_menu_button_get_gicon                   (GtdMenuButton      *self);


void                 gtd_menu_button_set_label                   (GtdMenuButton      *self,
                                                                  const gchar        *label);

const gchar*         gtd_menu_button_get_label                   (GtdMenuButton      *self);


void                 gtd_menu_button_set_use_underline           (GtdMenuButton      *self,
                                                                  gboolean            use_underline);

gboolean             gtd_menu_button_get_use_underline           (GtdMenuButton      *self);

void                 gtd_menu_button_set_has_frame               (GtdMenuButton      *self,
                                                                  gboolean            has_frame);

gboolean             gtd_menu_button_get_has_frame               (GtdMenuButton      *self);

void                 gtd_menu_button_popup                       (GtdMenuButton      *self);

void                 gtd_menu_button_popdown                     (GtdMenuButton      *self);


void                 gtd_menu_button_set_create_popup_func       (GtdMenuButton                *self,
                                                                  GtdMenuButtonCreatePopupFunc  func,
                                                                  gpointer                      user_data,
                                                                  GDestroyNotify                destroy_notify);


G_END_DECLS
