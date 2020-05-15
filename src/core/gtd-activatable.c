/* gtd-activatable.c
 *
 * Copyright (C) 2015-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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
 */

#define G_LOG_DOMAIN "GtdActivatable"

#include "gtd-activatable.h"
#include "gtd-panel.h"
#include "gtd-provider.h"

/**
 * SECTION:gtd-activatable
 * @short_description:entry point for plugins
 * @title:GtdActivatable
 * @stability:Unstable
 *
 * The #GtdActivatable interface is the interface plugins must
 * implement in order to be seen by GNOME To Do.
 *
 * When plugins are loaded, the gtd_activatable_activate() vfunc
 * is called. Use this vfunc to load anything that depends on GNOME
 * To Do.
 *
 * When plugins are unloaded, the gtd_activatable_deactivate() vfunc
 * if called. Ideally, the implementation should undo everything that
 * was done on gtd_activatable_activate().
 *
 * A plugin implementation may expose one or more #GtdProvider instances,
 * which are the data sources of GNOME To Do. See the 'eds' plugin for
 * a reference on how to expose one ('local') and multiple (Online Accounts)
 * providers.
 *
 * Plugins may also expose one or more #GtdPanel implementations.
 *
 * Optionally, a plugin may expose a preferences panel. See gtd_activatable_get_preferences_panel().
 */

G_DEFINE_INTERFACE (GtdActivatable, gtd_activatable, G_TYPE_OBJECT)

static void
gtd_activatable_default_init (GtdActivatableInterface *iface)
{
  /**
   * GtdActivatable::preferences-panel:
   *
   * The preferences panel of the plugin, or %NULL.
   */
  g_object_interface_install_property (iface,
                                       g_param_spec_object ("preferences-panel",
                                                            "Preferences panel",
                                                            "The preferences panel of the plugins",
                                                            GTK_TYPE_WIDGET,
                                                            G_PARAM_READABLE));
}

/**
 * gtd_activatable_activate:
 * @activatable: a #GtdActivatable
 *
 * Activates the extension. This is the starting point where
 * the implementation does everything it needs to do. Avoid
 * doing it earlier than this call.
 *
 * This function is called after the extension is loaded and
 * the signals are connected. If you want to do anything before
 * that, the _init function should be used instead.
 */
void
gtd_activatable_activate (GtdActivatable *activatable)
{
  g_return_if_fail (GTD_IS_ACTIVATABLE (activatable));

  if (GTD_ACTIVATABLE_GET_IFACE (activatable)->activate)
    GTD_ACTIVATABLE_GET_IFACE (activatable)->activate (activatable);
}

/**
 * gtd_activatable_deactivate:
 * @activatable: a #GtdActivatable
 *
 * Deactivates the extension. Here, the extension should remove
 * all providers and panels it set.
 *
 * This function is called before the extension is removed. At
 * this point, the plugin manager already removed all providers
 * and widgets this extension exported. If you want to do anything
 * after the extension is removed, use GObject::finalize instead.
 */
void
gtd_activatable_deactivate (GtdActivatable *activatable)
{
  g_return_if_fail (GTD_IS_ACTIVATABLE (activatable));

  if (GTD_ACTIVATABLE_GET_IFACE (activatable)->deactivate)
    GTD_ACTIVATABLE_GET_IFACE (activatable)->deactivate (activatable);
}

/**
 * gtd_activatable_get_preferences_panel:
 * @activatable: a #GtdActivatable
 *
 * Retrieve the preferences panel of @activatable if any.
 *
 * Returns: (transfer none)(nullable): a #GtkWidget, or %NULL
 */
GtkWidget*
gtd_activatable_get_preferences_panel (GtdActivatable *activatable)
{
  g_return_val_if_fail (GTD_IS_ACTIVATABLE (activatable), NULL);

  if (GTD_ACTIVATABLE_GET_IFACE (activatable)->get_preferences_panel)
    return GTD_ACTIVATABLE_GET_IFACE (activatable)->get_preferences_panel (activatable);

  return NULL;
}
