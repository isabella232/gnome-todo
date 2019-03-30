/* gtd-clock.c
 *
 * Copyright (C) 2017 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#define G_LOG_DOMAIN "GtdClock"

#include "gtd-clock.h"
#include "gtd-debug.h"

#include <gio/gio.h>

struct _GtdClock
{
  GtdObject           parent;

  guint               timeout_id;

  GDateTime          *current;

  GDBusProxy         *logind;
  GCancellable       *cancellable;
};

static gboolean      timeout_cb                                  (gpointer           user_data);

G_DEFINE_TYPE (GtdClock, gtd_clock, GTD_TYPE_OBJECT)

enum
{
  DAY_CHANGED,
  HOUR_CHANGED,
  MINUTE_CHANGED,
  UPDATE,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0, };

/*
 * Auxiliary methods
 */

static void
update_current_date (GtdClock *self)
{
  g_autoptr (GDateTime) now;
  gboolean minute_changed;
  gboolean hour_changed;
  gboolean day_changed;

  GTD_ENTRY;

  now = g_date_time_new_now_local ();

  day_changed = g_date_time_get_year (now) != g_date_time_get_year (self->current) ||
                g_date_time_get_day_of_year (now) != g_date_time_get_day_of_year (self->current);
  hour_changed = day_changed || g_date_time_get_hour (now) != g_date_time_get_hour (self->current);
  minute_changed = hour_changed || g_date_time_get_minute (now) != g_date_time_get_minute (self->current);

  if (day_changed)
    g_signal_emit (self, signals[DAY_CHANGED], 0);

  if (hour_changed)
    g_signal_emit (self, signals[HOUR_CHANGED], 0);

  if (minute_changed)
    g_signal_emit (self, signals[MINUTE_CHANGED], 0);

  g_signal_emit (self, signals[UPDATE], 0);

  GTD_TRACE_MSG ("Ticking clock");

  g_clear_pointer (&self->current, g_date_time_unref);
  self->current = g_date_time_ref (now);

  GTD_EXIT;
}

static void
schedule_update (GtdClock *self)
{
  g_autoptr (GDateTime) now;
  guint seconds_between;

  /* Remove the previous timeout if we came from resume */
  if (self->timeout_id > 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  now = g_date_time_new_now_local ();

  seconds_between = 60 - g_date_time_get_second (now);

  self->timeout_id = g_timeout_add_seconds (seconds_between, timeout_cb, self);
}

/*
 * Callbacks
 */

static void
logind_signal_received_cb (GDBusProxy  *logind,
                           const gchar *sender,
                           const gchar *signal,
                           GVariant    *params,
                           GtdClock    *self)
{
  GVariant *child;
  gboolean resuming;

  if (!g_str_equal (signal, "PrepareForSleep"))
    return;

  child = g_variant_get_child_value (params, 0);
  resuming = !g_variant_get_boolean (child);

  /* Only emit :update when resuming */
  if (resuming)
    {
      /* Reschedule the daily timeout */
      update_current_date (self);
      schedule_update (self);
    }

  g_clear_pointer (&child, g_variant_unref);
}

static void
login_proxy_acquired_cb (GObject      *source,
                         GAsyncResult *res,
                         gpointer      user_data)
{
  g_autoptr (GError) error = NULL;
  GtdClock *self;

  self = GTD_CLOCK (user_data);

  gtd_object_pop_loading (GTD_OBJECT (self));

  self->logind = g_dbus_proxy_new_for_bus_finish (res, &error);

  if (error)
    {
      g_warning ("Error acquiring org.freedesktop.login1: %s", error->message);
      return;
    }

  g_signal_connect (self->logind,
                    "g-signal",
                    G_CALLBACK (logind_signal_received_cb),
                    self);
}

static gboolean
timeout_cb (gpointer user_data)
{
  GtdClock *self = user_data;

  self->timeout_id = 0;

  update_current_date (self);
  schedule_update (self);

  return G_SOURCE_REMOVE;
}


/*
 * GObject overrides
 */
static void
gtd_clock_finalize (GObject *object)
{
  GtdClock *self = (GtdClock *)object;

  g_cancellable_cancel (self->cancellable);

  if (self->timeout_id > 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  g_clear_pointer (&self->current, g_date_time_unref);

  g_clear_object (&self->cancellable);
  g_clear_object (&self->logind);

  G_OBJECT_CLASS (gtd_clock_parent_class)->finalize (object);
}

static void
gtd_clock_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void
gtd_clock_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void
gtd_clock_class_init (GtdClockClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_clock_finalize;
  object_class->get_property = gtd_clock_get_property;
  object_class->set_property = gtd_clock_set_property;

  /**
   * GtdClock:day-changed:
   *
   * Emited when the day changes.
   */
  signals[DAY_CHANGED] = g_signal_new ("day-changed",
                                       GTD_TYPE_CLOCK,
                                       G_SIGNAL_RUN_LAST,
                                       0, NULL, NULL, NULL,
                                       G_TYPE_NONE,
                                       0);
  /**
   * GtdClock:hour-changed:
   *
   * Emited when the current hour changes.
   */
  signals[HOUR_CHANGED] = g_signal_new ("hour-changed",
                                        GTD_TYPE_CLOCK,
                                        G_SIGNAL_RUN_LAST,
                                        0, NULL, NULL, NULL,
                                        G_TYPE_NONE,
                                        0);
  /**
   * GtdClock:minute-changed:
   *
   * Emited when the current minute changes.
   */
  signals[MINUTE_CHANGED] = g_signal_new ("minute-changed",
                                          GTD_TYPE_CLOCK,
                                          G_SIGNAL_RUN_LAST,
                                          0, NULL, NULL, NULL,
                                          G_TYPE_NONE,
                                          0);

  /**
   * GtdClock:update:
   *
   * Emited when an update is required. This is emited usually
   * after a session resume, or a day change.
   */
  signals[UPDATE] = g_signal_new ("update",
                                  GTD_TYPE_CLOCK,
                                  G_SIGNAL_RUN_LAST,
                                  0, NULL, NULL, NULL,
                                  G_TYPE_NONE,
                                  0);
}

static void
gtd_clock_init (GtdClock *self)
{
  gtd_object_push_loading (GTD_OBJECT (self));

  self->current = g_date_time_new_now_local ();
  self->cancellable = g_cancellable_new ();

  g_dbus_proxy_new_for_bus (G_BUS_TYPE_SYSTEM,
                            G_DBUS_PROXY_FLAGS_NONE,
                            NULL,
                            "org.freedesktop.login1",
                            "/org/freedesktop/login1",
                            "org.freedesktop.login1.Manager",
                            self->cancellable,
                            login_proxy_acquired_cb,
                            self);

  schedule_update (self);
}

GtdClock*
gtd_clock_new (void)
{
  return g_object_new (GTD_TYPE_CLOCK, NULL);
}
