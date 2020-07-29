/* gtd-peace-omni-area-addin.c
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

#include "gtd-peace-omni-area-addin.h"

#include "config.h"

#include <glib/gi18n.h>

#define MESSAGE_ID "peace-message-id"

#define SWITCH_MESSAGE_TIMEOUT 20 * 60
#define REMOVE_MESSAGE_TIMEOUT 1 * 60

struct _GtdPeaceOmniAreaAddin
{
  GObject             parent;

  GtdOmniArea        *omni_area;

  guint               timeout_id;
};

static gboolean      switch_message_cb                           (gpointer                   user_data);

static void          gtd_omni_area_addin_iface_init              (GtdOmniAreaAddinInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtdPeaceOmniAreaAddin, gtd_peace_omni_area_addin, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTD_TYPE_OMNI_AREA_ADDIN, gtd_omni_area_addin_iface_init))

typedef struct
{
  const gchar *text;
  const gchar *icon_name;
} str_pair;

const str_pair mindful_questions[] =
{
  { N_("Did you drink some water today?"), NULL },
  { N_("What are your goals for today?"), NULL },
  { N_("Can you let your creativity flow?"), NULL },
  { N_("How are you feeling right now?"), NULL },
  { N_("At what point is it good enough?"), NULL },
};

const str_pair reminders[] =
{
  { N_("Remember to breathe. Good. Don't stop."), NULL },
  { N_("Don't forget to drink some water"), NULL },
  { N_("Remember to take some time off"), NULL },
  { N_("Eat fruits if you can 🍐️"), NULL },
  { N_("Take care of yourself"), NULL },
  { N_("Remember to have some fun"), NULL },
  { N_("You're doing great"), NULL },
};

const str_pair inspiring_quotes[] =
{
  { N_("Smile, breathe and go slowly"), NULL },
  { N_("Wherever you go, there you are"), NULL },
  { N_("Working hard is always rewarded"), NULL },
  { N_("Keep calm"), NULL },
  { N_("You can do it"), NULL },
  { N_("Meanwhile, spread the love ♥️"), NULL },
};


/*
 * Callbacks
 */

static gboolean
remove_message_cb (gpointer user_data)
{
  GtdPeaceOmniAreaAddin *self = GTD_PEACE_OMNI_AREA_ADDIN (user_data);
  gint factor = g_random_int_range (2, 6);

  gtd_omni_area_withdraw_message (self->omni_area, MESSAGE_ID);

  self->timeout_id = g_timeout_add_seconds (SWITCH_MESSAGE_TIMEOUT * factor, switch_message_cb, self);

  return G_SOURCE_REMOVE;
}

static gboolean
switch_message_cb (gpointer user_data)
{
  GtdPeaceOmniAreaAddin *self;
  g_autoptr (GIcon) icon = NULL;
  const gchar *message;
  const gchar *icon_name;
  gint source;

  self = GTD_PEACE_OMNI_AREA_ADDIN (user_data);
  source = g_random_int_range (0, 3);

  if (source == 0)
    {
      gint i = g_random_int_range (0, G_N_ELEMENTS (mindful_questions));

      message = gettext (mindful_questions[i].text);
      icon_name = mindful_questions[i].icon_name;
    }
  else if (source == 1)
    {
      gint i = g_random_int_range (0, G_N_ELEMENTS (reminders));

      message = gettext (reminders[i].text);
      icon_name = reminders[i].icon_name;
    }
  else
    {
      gint i = g_random_int_range (0, G_N_ELEMENTS (inspiring_quotes));

      message = gettext (inspiring_quotes[i].text);
      icon_name = inspiring_quotes[i].icon_name;
    }

  if (icon_name)
    icon = g_themed_icon_new (icon_name);

  gtd_omni_area_withdraw_message (self->omni_area, MESSAGE_ID);
  gtd_omni_area_push_message (self->omni_area, MESSAGE_ID, message, NULL);

  self->timeout_id = g_timeout_add_seconds (REMOVE_MESSAGE_TIMEOUT, remove_message_cb, self);

  return G_SOURCE_REMOVE;
}


/*
 * GtdOmniAreaAddin iface
 */

static void
gtd_today_omni_area_addin_omni_area_addin_load (GtdOmniAreaAddin *addin,
                                                GtdOmniArea      *omni_area)
{
  GtdPeaceOmniAreaAddin *self = GTD_PEACE_OMNI_AREA_ADDIN (addin);

  self->omni_area = omni_area;

  g_clear_handle_id (&self->timeout_id, g_source_remove);
  self->timeout_id = g_timeout_add_seconds (SWITCH_MESSAGE_TIMEOUT, switch_message_cb, self);
}

static void
gtd_today_omni_area_addin_omni_area_addin_unload (GtdOmniAreaAddin *addin,
                                                  GtdOmniArea      *omni_area)
{
  GtdPeaceOmniAreaAddin *self = GTD_PEACE_OMNI_AREA_ADDIN (addin);

  gtd_omni_area_withdraw_message (omni_area, MESSAGE_ID);

  g_clear_handle_id (&self->timeout_id, g_source_remove);
  self->omni_area = NULL;
}

static void
gtd_omni_area_addin_iface_init (GtdOmniAreaAddinInterface *iface)
{
  iface->load = gtd_today_omni_area_addin_omni_area_addin_load;
  iface->unload = gtd_today_omni_area_addin_omni_area_addin_unload;
}


/*
 * GObject overrides
 */

static void
gtd_peace_omni_area_addin_finalize (GObject *object)
{
  GtdPeaceOmniAreaAddin *self = (GtdPeaceOmniAreaAddin *)object;

  g_clear_handle_id (&self->timeout_id, g_source_remove);

  G_OBJECT_CLASS (gtd_peace_omni_area_addin_parent_class)->finalize (object);
}

static void
gtd_peace_omni_area_addin_class_init (GtdPeaceOmniAreaAddinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_peace_omni_area_addin_finalize;
}

static void
gtd_peace_omni_area_addin_init (GtdPeaceOmniAreaAddin *self)
{
}
