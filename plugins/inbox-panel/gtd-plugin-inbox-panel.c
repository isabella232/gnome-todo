/* gtd-plugin-inbox-panel.c
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

#define G_LOG_DOMAIN "GtdPluginInboxPanel"

#include "gtd-inbox-panel.h"

#include "gtd-plugin-inbox-panel.h"

#include <glib/gi18n.h>
#include <glib-object.h>

struct _GtdPluginInboxPanel
{
  PeasExtensionBase   parent;

  GtkCssProvider     *css_provider;
};

static void          gtd_activatable_iface_init                  (GtdActivatableInterface  *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GtdPluginInboxPanel, gtd_plugin_inbox_panel, PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (GTD_TYPE_ACTIVATABLE,
                                                               gtd_activatable_iface_init))

enum
{
  PROP_0,
  PROP_PREFERENCES_PANEL,
  N_PROPS
};

/*
 * GtdActivatable interface implementation
 */
static void
gtd_plugin_inbox_panel_activate (GtdActivatable *activatable)
{
  ;
}

static void
gtd_plugin_inbox_panel_deactivate (GtdActivatable *activatable)
{
  ;
}

static GList*
gtd_plugin_inbox_panel_get_header_widgets (GtdActivatable *activatable)
{
  return NULL;
}

static GtkWidget*
gtd_plugin_inbox_panel_get_preferences_panel (GtdActivatable *activatable)
{
  return NULL;
}

static GList*
gtd_plugin_inbox_panel_get_providers (GtdActivatable *activatable)
{
  return NULL;
}

static void
gtd_activatable_iface_init (GtdActivatableInterface *iface)
{
  iface->activate = gtd_plugin_inbox_panel_activate;
  iface->deactivate = gtd_plugin_inbox_panel_deactivate;
  iface->get_header_widgets = gtd_plugin_inbox_panel_get_header_widgets;
  iface->get_preferences_panel = gtd_plugin_inbox_panel_get_preferences_panel;
  iface->get_providers = gtd_plugin_inbox_panel_get_providers;
}

static void
gtd_plugin_inbox_panel_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_PREFERENCES_PANEL:
      g_value_set_object (value, NULL);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_plugin_inbox_panel_class_init (GtdPluginInboxPanelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gtd_plugin_inbox_panel_get_property;

  g_object_class_override_property (object_class, PROP_PREFERENCES_PANEL, "preferences-panel");
}

static void
gtd_plugin_inbox_panel_init (GtdPluginInboxPanel *self)
{
}

static void
gtd_plugin_inbox_panel_class_finalize (GtdPluginInboxPanelClass *klass)
{
}

G_MODULE_EXPORT void
gtd_plugin_inbox_panel_register_types (PeasObjectModule *module)
{
  gtd_plugin_inbox_panel_register_type (G_TYPE_MODULE (module));

  peas_object_module_register_extension_type (module,
                                              GTD_TYPE_ACTIVATABLE,
                                              GTD_TYPE_PLUGIN_INBOX_PANEL);

  peas_object_module_register_extension_type (module,
                                              GTD_TYPE_PANEL,
                                              GTD_TYPE_INBOX_PANEL);
}
