/* gtd-task-list-eds.h
 *
 * Copyright (C) 2015 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#include "e-source-gnome-todo.h"

struct _ESourceGnomeTodo
{
  ESourceExtension    parent;

  guint               api_version;
};

G_DEFINE_TYPE (ESourceGnomeTodo, e_source_gnome_todo, E_TYPE_SOURCE_EXTENSION)

enum
{
  PROP_0,
  PROP_API_VERSION,
  N_PROPS
};

static GParamSpec *properties [N_PROPS] = { NULL, };


/*
 * GObject overrides
 */

static void
e_source_gnome_todo_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  ESourceGnomeTodo *self = E_SOURCE_GNOME_TODO (object);

  switch (prop_id)
    {
    case PROP_API_VERSION:
      g_value_set_uint (value, self->api_version);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
e_source_gnome_todo_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  ESourceGnomeTodo *self = E_SOURCE_GNOME_TODO (object);

  switch (prop_id)
    {
    case PROP_API_VERSION:
      self->api_version = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
e_source_gnome_todo_class_init (ESourceGnomeTodoClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  ESourceExtensionClass *extension_class = E_SOURCE_EXTENSION_CLASS (klass);

  object_class->get_property = e_source_gnome_todo_get_property;
  object_class->set_property = e_source_gnome_todo_set_property;

  extension_class->name = E_SOURCE_EXTENSION_GNOME_TODO;

  properties[PROP_API_VERSION] = g_param_spec_uint ("api-version",
                                                    "API Version",
                                                    "API Version",
                                                    0,
                                                    G_MAXUINT,
                                                    0,
                                                    G_PARAM_READWRITE | E_SOURCE_PARAM_SETTING | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
e_source_gnome_todo_init (ESourceGnomeTodo *self)
{
  self->api_version = 0;
}

guint
e_source_gnome_todo_get_api_version (ESourceGnomeTodo *self)
{
  g_return_val_if_fail (E_IS_SOURCE_GNOME_TODO (self), 0);

  return self->api_version;
}

void
e_source_gnome_todo_set_api_version (ESourceGnomeTodo *self,
                                     guint             api_version)
{
  g_return_if_fail (E_IS_SOURCE_GNOME_TODO (self));

  e_source_extension_property_lock (E_SOURCE_EXTENSION (self));
  self->api_version = api_version;
  e_source_extension_property_unlock (E_SOURCE_EXTENSION (self));

  g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_API_VERSION]);
}
