/* gtd-today-omni-area-addin.c
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

#include "gtd-today-omni-area-addin.h"

#include "gnome-todo.h"
#include "config.h"

#include <glib/gi18n.h>

#define MESSAGE_ID "today-counter-message-id"

struct _GtdTodayOmniAreaAddin
{
  GObject             parent;

  GIcon              *icon;
  GtkFilterListModel *filter_model;

  GtdOmniArea        *omni_area;
  guint               number_of_tasks;

  gboolean            had_tasks;
  gboolean            finished_tasks;

  guint               idle_update_message_timeout_id;
};

static void          gtd_omni_area_addin_iface_init              (GtdOmniAreaAddinInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtdTodayOmniAreaAddin, gtd_today_omni_area_addin, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (GTD_TYPE_OMNI_AREA_ADDIN, gtd_omni_area_addin_iface_init))

const gchar *end_messages[] =
{
  N_("No more tasks left"),
  N_("Nothing else to do here"),
  N_("You made it!"),
  N_("Looks like there’s nothing else left here")
};

static void
update_omni_area_message (GtdTodayOmniAreaAddin *self)
{
  g_autofree gchar *message = NULL;

  g_assert (self->omni_area != NULL);

  if (self->number_of_tasks > 0)
    {
      message = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                              "%d task for today",
                                              "%d tasks for today",
                                              self->number_of_tasks),
                                 self->number_of_tasks);
    }
  else
    {
      if (self->finished_tasks)
        {
          gint message_index = g_random_int_range (0, G_N_ELEMENTS (end_messages));

          message = g_strdup (gettext (end_messages[message_index]));
        }
      else
        {
          message = g_strdup (_("No tasks scheduled for today"));
        }
    }

  gtd_omni_area_withdraw_message (self->omni_area, MESSAGE_ID);
  gtd_omni_area_push_message (self->omni_area, MESSAGE_ID, message, self->icon);
}

static gboolean
is_today (GDateTime *today,
          GDateTime *dt)
{
  if (!dt)
    return FALSE;

  if (g_date_time_get_year (dt) == g_date_time_get_year (today) &&
      g_date_time_get_day_of_year (dt) == g_date_time_get_day_of_year (today))
    {
      return TRUE;
    }

  return FALSE;
}


/*
 * Callbacks
 */

static gboolean
idle_update_omni_area_message_cb (gpointer user_data)
{
  GtdTodayOmniAreaAddin *self = GTD_TODAY_OMNI_AREA_ADDIN (user_data);

  update_omni_area_message (self);

  self->idle_update_message_timeout_id = 0;

  return G_SOURCE_REMOVE;
}

static gboolean
filter_func (gpointer  item,
             gpointer  user_data)
{
  g_autoptr (GDateTime) task_dt = NULL;
  g_autoptr (GDateTime) now = NULL;
  GtdTask *task;

  task = (GtdTask*) item;

  if (gtd_task_get_complete (task))
    return FALSE;

  now = g_date_time_new_now_local ();
  task_dt = gtd_task_get_due_date (task);

  return is_today (now, task_dt);
}

static void
on_clock_day_changed_cb (GtdClock              *clock,
                         GtdTodayOmniAreaAddin *self)
{
  self->had_tasks = FALSE;
  self->finished_tasks = FALSE;

  gtk_filter_list_model_refilter (self->filter_model);
}

static void
on_model_items_changed_cb (GListModel            *model,
                           guint                  position,
                           guint                  n_removed,
                           guint                  n_added,
                           GtdTodayOmniAreaAddin *self)
{
  guint number_of_tasks = g_list_model_get_n_items (model);

  if (self->number_of_tasks == number_of_tasks)
    return;

  self->number_of_tasks = number_of_tasks;

  if (number_of_tasks != 0)
    self->had_tasks = number_of_tasks != 0;

  self->finished_tasks = self->had_tasks && number_of_tasks == 0;

  g_clear_handle_id (&self->idle_update_message_timeout_id, g_source_remove);
  self->idle_update_message_timeout_id = g_timeout_add_seconds (2, idle_update_omni_area_message_cb, self);
}


/*
 * GtdOmniAreaAddin iface
 */

static void
gtd_today_omni_area_addin_omni_area_addin_load (GtdOmniAreaAddin *addin,
                                                GtdOmniArea      *omni_area)
{
  GtdTodayOmniAreaAddin *self = GTD_TODAY_OMNI_AREA_ADDIN (addin);

  self->omni_area = omni_area;
  update_omni_area_message (self);
}

static void
gtd_today_omni_area_addin_omni_area_addin_unload (GtdOmniAreaAddin *addin,
                                                  GtdOmniArea      *omni_area)
{
  GtdTodayOmniAreaAddin *self = GTD_TODAY_OMNI_AREA_ADDIN (addin);

  gtd_omni_area_withdraw_message (omni_area, MESSAGE_ID);
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
gtd_today_omni_area_addin_finalize (GObject *object)
{
  GtdTodayOmniAreaAddin *self = (GtdTodayOmniAreaAddin *)object;

  g_clear_handle_id (&self->idle_update_message_timeout_id, g_source_remove);
  g_clear_object (&self->icon);
  g_clear_object (&self->filter_model);

  G_OBJECT_CLASS (gtd_today_omni_area_addin_parent_class)->finalize (object);
}

static void
gtd_today_omni_area_addin_class_init (GtdTodayOmniAreaAddinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_today_omni_area_addin_finalize;
}

static void
gtd_today_omni_area_addin_init (GtdTodayOmniAreaAddin *self)
{
  GtdManager *manager;

  manager = gtd_manager_get_default ();

  self->icon = g_themed_icon_new ("view-tasks-today-symbolic");
  self->filter_model = gtk_filter_list_model_new (gtd_manager_get_tasks_model (manager), filter_func, self, NULL);

  g_signal_connect_object (self->filter_model,
                           "items-changed",
                           G_CALLBACK (on_model_items_changed_cb),
                           self,
                           0);

  g_signal_connect_object (gtd_manager_get_clock (manager),
                           "day-changed",
                           G_CALLBACK (on_clock_day_changed_cb),
                           self,
                           0);
}
