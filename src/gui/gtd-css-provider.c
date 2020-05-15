/* gtd-css-provider.c
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

#define G_LOG_DOMAIN "GtdCssProvider"

#include "gtd-css-provider.h"

struct _GtdCssProvider
{
  GObject             parent_instance;

  GtkCssProvider     *provider;
  gchar              *base_path;
  guint               queued_update;
};

G_DEFINE_TYPE (GtdCssProvider, gtd_css_provider, G_TYPE_OBJECT)

enum
{
  PROP_0,
  PROP_BASE_PATH,
  N_PROPS,
};

static GParamSpec *properties [N_PROPS];

static gboolean
resource_exists (const gchar *resource_path)
{
  g_assert (resource_path != NULL);

  if (g_str_has_prefix (resource_path, "resource://"))
    {
      gsize len = 0;
      guint32 flags = 0;

      resource_path += strlen ("resource://");

      return g_resources_get_info (resource_path, G_RESOURCE_LOOKUP_FLAGS_NONE, &len, &flags, NULL);
    }

  return g_file_test (resource_path, G_FILE_TEST_IS_REGULAR);
}

static void
load_resource (GtdCssProvider *self,
               const gchar    *resource_path)
{
  g_assert (resource_path != NULL);

  if (g_str_has_prefix (resource_path, "resource://"))
    {
      resource_path += strlen ("resource://");
      gtk_css_provider_load_from_resource (self->provider, resource_path);
    }
  else
    {
      gtk_css_provider_load_from_path (self->provider, resource_path);
    }

}

static void
update_css_provider (GtdCssProvider *self)
{
  g_autofree gchar *theme_name = NULL;
  g_autofree gchar *resource_path = NULL;
  GtkSettings *settings;
  gboolean prefer_dark_theme = FALSE;

  settings = gtk_settings_get_default ();
  theme_name = g_strdup (g_getenv ("GTK_THEME"));

  if (theme_name != NULL)
    {
      char *p;

      /* Theme variants are specified with the syntax
       * "<theme>:<variant>" e.g. "Adwaita:dark" */
      if (NULL != (p = strrchr (theme_name, ':')))
        {
          *p = '\0';
          p++;
          prefer_dark_theme = g_strcmp0 (p, "dark") == 0;
        }
    }
  else
    {
      g_object_get (settings,
                    "gtk-theme-name", &theme_name,
                    "gtk-application-prefer-dark-theme", &prefer_dark_theme,
                    NULL);
    }

  /* First check with full path to theme+variant */
  resource_path = g_strdup_printf ("%s/%s%s.css",
                                   self->base_path,
                                   theme_name, prefer_dark_theme ? "-dark" : "");

  if (!resource_exists (resource_path))
    {
      /* Now try without the theme variant */
      g_free (resource_path);
      resource_path = g_strdup_printf ("%s/%s.css", self->base_path, theme_name);

      /* Now fallback to shared styling */
      if (!resource_exists (resource_path))
        {
          g_free (resource_path);
          resource_path = g_strdup_printf ("%s/shared.css", self->base_path);

          if (!resource_exists (resource_path))
            return;
        }
    }

  g_debug ("Loading css overrides \"%s\"", resource_path);

  load_resource (self, resource_path);
}

static gboolean
do_update_cb (gpointer user_data)
{
  GtdCssProvider *self = user_data;

  g_assert (GTD_IS_CSS_PROVIDER (self));

  self->queued_update = 0;
  update_css_provider (self);

  return G_SOURCE_REMOVE;
}

static void
dzl_css_provider_queue_update (GtdCssProvider *self)
{
  if (self->queued_update > 0)
    return;


  self->queued_update = g_idle_add_full (G_PRIORITY_LOW,
                                         do_update_cb,
                                         g_object_ref (self),
                                         g_object_unref);
}

static void
on_settings_notify_gtk_theme_name_cb (GtdCssProvider *self,
                                      GParamSpec     *pspec,
                                      GtkSettings    *settings)
{
  dzl_css_provider_queue_update (self);
}

static void
on_settings_notify_gtk_application_prefer_dark_theme_cb (GtdCssProvider *self,
                                                         GParamSpec     *pspec,
                                                         GtkSettings    *settings)
{
  dzl_css_provider_queue_update (self);
}

static void
gtd_css_provider_constructed (GObject *object)
{
  GtdCssProvider *self = (GtdCssProvider *)object;
  GtkSettings *settings;

  G_OBJECT_CLASS (gtd_css_provider_parent_class)->constructed (object);

  settings = gtk_settings_get_default ();

  g_signal_connect_object (settings,
                           "notify::gtk-theme-name",
                           G_CALLBACK (on_settings_notify_gtk_theme_name_cb),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (settings,
                           "notify::gtk-application-prefer-dark-theme",
                           G_CALLBACK (on_settings_notify_gtk_application_prefer_dark_theme_cb),
                           self,
                           G_CONNECT_SWAPPED);

  update_css_provider (self);
}

static void
gtd_css_provider_finalize (GObject *object)
{
  GtdCssProvider *self = (GtdCssProvider *)object;

  g_clear_handle_id (&self->queued_update, g_source_remove);
  g_clear_pointer (&self->base_path, g_free);
  g_clear_object (&self->provider);

  G_OBJECT_CLASS (gtd_css_provider_parent_class)->finalize (object);
}

static void
gtd_css_provider_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GtdCssProvider *self = GTD_CSS_PROVIDER(object);

  switch (prop_id)
    {
    case PROP_BASE_PATH:
      g_value_set_string (value, self->base_path);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
gtd_css_provider_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GtdCssProvider *self = GTD_CSS_PROVIDER(object);

  switch (prop_id)
    {
    case PROP_BASE_PATH:
      {
        const gchar *str = g_value_get_string (value);
        gsize len = str ? strlen (str) : 0;

        /* Ignore trailing slash to simplify building paths */
        if (str && len && str[len-1] == '/')
          self->base_path = g_strndup (str, len - 1);
        else
          self->base_path = g_strdup (str);

        break;
      }

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
gtd_css_provider_class_init (GtdCssProviderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gtd_css_provider_constructed;
  object_class->finalize = gtd_css_provider_finalize;
  object_class->get_property = gtd_css_provider_get_property;
  object_class->set_property = gtd_css_provider_set_property;

  properties [PROP_BASE_PATH] =
    g_param_spec_string ("base-path",
                         "Base Path",
                         "The base resource path to discover themes",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gtd_css_provider_init (GtdCssProvider *self)
{
  self->provider = gtk_css_provider_new ();
}

GtdCssProvider *
gtd_css_provider_new (const gchar *base_path)
{
  return g_object_new (GTD_TYPE_CSS_PROVIDER,
                       "base-path", base_path,
                       NULL);
}

GtkCssProvider*
gtd_css_provider_get_provider (GtdCssProvider *self)
{
  g_return_val_if_fail (GTD_IS_CSS_PROVIDER (self), NULL);

  return self->provider;
}
