/* gtd-theme-manager.c
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

#define G_LOG_DOMAIN "GtdThemeManager"

#include "gtd-theme-manager.h"

#include "gtd-css-provider.h"

#include <gtk/gtk.h>

struct _GtdThemeManager
{
  GObject             parent_instance;

  GHashTable         *providers_by_path;
};

G_DEFINE_TYPE (GtdThemeManager, gtd_theme_manager, G_TYPE_OBJECT)


/*
 * Auxiliary methods
 */

static gboolean
has_child_resources (const gchar *path)
{
  g_auto(GStrv) children = NULL;

  if (g_str_has_prefix (path, "resource://"))
    path += strlen ("resource://");

  children = g_resources_enumerate_children (path, 0, NULL);

  return children != NULL && children[0] != NULL;
}


/*
 * GObject overrides
 */

static void
gtd_theme_manager_finalize (GObject *object)
{
  GtdThemeManager *self = (GtdThemeManager *)object;

  g_clear_pointer (&self->providers_by_path, g_hash_table_unref);

  G_OBJECT_CLASS (gtd_theme_manager_parent_class)->finalize (object);
}

static void
gtd_theme_manager_class_init (GtdThemeManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_theme_manager_finalize;
}

static void
gtd_theme_manager_init (GtdThemeManager *self)
{
  self->providers_by_path = g_hash_table_new_full (g_str_hash,
                                                   g_str_equal,
                                                   g_free,
                                                   g_object_unref);
}

GtdThemeManager*
gtd_theme_manager_new (void)
{
  return g_object_new (GTD_TYPE_THEME_MANAGER, NULL);
}

void
gtd_theme_manager_add_resources (GtdThemeManager *self,
                                 const gchar     *resource_path)
{
  g_autofree gchar *css_dir = NULL;
  g_autofree gchar *icons_dir = NULL;
  const gchar *real_path = resource_path;
  GtkIconTheme *theme;

  g_return_if_fail (GTD_IS_THEME_MANAGER (self));
  g_return_if_fail (resource_path != NULL);

  theme = gtk_icon_theme_get_for_display (gdk_display_get_default ());

  if (g_str_has_prefix (real_path, "resource://"))
    real_path += strlen ("resource://");

  /*
   * Create a CSS provider that will load the proper variant based on the
   * current application theme, using @resource_path/css as the base directory
   * to locate theming files.
   */
  css_dir = g_build_path ("/", resource_path, "themes/", NULL);

  g_debug ("Including CSS overrides from %s", css_dir);

  if (has_child_resources (css_dir))
    {
      GtdCssProvider *css_provider;
      GtkCssProvider *provider;

      css_provider = gtd_css_provider_new (css_dir);
      provider = gtd_css_provider_get_provider (css_provider);

      g_hash_table_insert (self->providers_by_path,
                           g_strdup (resource_path),
                           g_object_ref (css_provider));
      gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                  GTK_STYLE_PROVIDER (provider),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

  /*
   * Add the icons sub-directory so that Gtk can locate the themed
   * icons (svg, png, etc).
   */
  icons_dir = g_build_path ("/", real_path, "icons/", NULL);

  g_debug ("Loading icon resources from %s", icons_dir);

  if (!g_str_equal (real_path, resource_path))
    {
      g_auto (GStrv) children = NULL;

      /* Okay, this is a resource-based path. Make sure the
       * path contains children so we don't slow down the
       * theme loading code with tons of useless directories.
       */
      children = g_resources_enumerate_children (icons_dir, 0, NULL);
      if (children != NULL && children[0] != NULL)
        gtk_icon_theme_add_resource_path (theme, icons_dir);
    }
  else
    {
      /* Make sure the directory exists so that we don't needlessly
       * slow down the icon loading paths.
       */
      if (g_file_test (icons_dir, G_FILE_TEST_IS_DIR))
        gtk_icon_theme_add_search_path (theme, icons_dir);
    }
}

void
gtd_theme_manager_remove_resources (GtdThemeManager *self,
                                    const gchar     *resource_path)
{
  GtdCssProvider *css_provider;

  g_return_if_fail (GTD_IS_THEME_MANAGER (self));
  g_return_if_fail (resource_path != NULL);

  css_provider = g_hash_table_lookup (self->providers_by_path, resource_path);

  if (css_provider)
    {
      GtkCssProvider *provider = gtd_css_provider_get_provider (css_provider);

      g_debug ("Removing CSS overrides from %s", resource_path);

      gtk_style_context_remove_provider_for_display (gdk_display_get_default (),
                                                     GTK_STYLE_PROVIDER (provider));
      g_hash_table_remove (self->providers_by_path, resource_path);
    }
}
