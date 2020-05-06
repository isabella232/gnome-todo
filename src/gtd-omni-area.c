/* gtd-omni-area.c
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

#define G_LOG_DOMAIN "GtdOmniArea"

#include "gtd-omni-area.h"

#include "gtd-debug.h"
#include "gtd-omni-area-addin.h"
#include "gtd-text-width-layout.h"

#include <libpeas/peas.h>

struct _GtdOmniArea
{
  GtdWidget           parent;

  GtkStack           *main_stack;
  GtkStack           *status_stack;

  PeasExtensionSet   *addins;

  GQueue             *messages;
  guint               current;

  guint               switch_messages_timeout_id;
};

G_DEFINE_TYPE (GtdOmniArea, gtd_omni_area, GTD_TYPE_WIDGET)


/*
 * Auxiliary methods
 */

static void
show_message (GtdOmniArea *self,
              guint        message_index)
{
  const gchar *message_id;

  message_id = g_queue_peek_nth (self->messages, message_index);

  gtk_stack_set_visible_child_name (self->status_stack, message_id);
  self->current = message_index;
}



/*
 * Callbacks
 */

static gboolean
switch_message_cb (gpointer user_data)
{
  GtdOmniArea *self = GTD_OMNI_AREA (user_data);
  gint next_message_index;
  guint n_messages;

  n_messages = g_queue_get_length (self->messages);
  gtk_stack_set_visible_child_name (self->main_stack, n_messages > 0 ? "messages" : "placeholder");

  next_message_index = (self->current + 1) % n_messages;
  show_message (self, next_message_index);

  return G_SOURCE_CONTINUE;
}

static void
on_omni_area_addin_added_cb (PeasExtensionSet *extension_set,
                             PeasPluginInfo   *plugin_info,
                             GtdOmniAreaAddin *addin,
                             GtdOmniArea      *self)
{
  gtd_omni_area_addin_load (addin, self);
}

static void
on_omni_area_addin_removed_cb (PeasExtensionSet *extension_set,
                               PeasPluginInfo   *plugin_info,
                               GtdOmniAreaAddin *addin,
                               GtdOmniArea      *self)
{
  gtd_omni_area_addin_unload (addin, self);
}


/*
 * GObject overrides
 */

static void
gtd_omni_area_dispose (GObject *object)
{
  GtdOmniArea *self = GTD_OMNI_AREA (object);

  g_clear_object (&self->addins);

  G_OBJECT_CLASS (gtd_omni_area_parent_class)->dispose (object);
}

static void
gtd_omni_area_finalize (GObject *object)
{
  GtdOmniArea *self = (GtdOmniArea *)object;

  g_clear_handle_id (&self->switch_messages_timeout_id, g_source_remove);
  g_queue_free_full (self->messages, g_free);

  G_OBJECT_CLASS (gtd_omni_area_parent_class)->finalize (object);
}

static void
gtd_omni_area_class_init (GtdOmniAreaClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = gtd_omni_area_dispose;
  object_class->finalize = gtd_omni_area_finalize;

  g_type_ensure (GTD_TYPE_TEXT_WIDTH_LAYOUT);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/gtd-omni-area.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdOmniArea, main_stack);
  gtk_widget_class_bind_template_child (widget_class, GtdOmniArea, status_stack);

  gtk_widget_class_set_css_name (widget_class, "omniarea");
}

static void
gtd_omni_area_init (GtdOmniArea *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->messages = g_queue_new ();
  self->addins = peas_extension_set_new (peas_engine_get_default (),
                                         GTD_TYPE_OMNI_AREA_ADDIN,
                                         NULL);

  peas_extension_set_foreach (self->addins,
                              (PeasExtensionSetForeachFunc) on_omni_area_addin_added_cb,
                              self);

  g_signal_connect (self->addins, "extension-added", G_CALLBACK (on_omni_area_addin_added_cb), self);
  g_signal_connect (self->addins, "extension-removed", G_CALLBACK (on_omni_area_addin_removed_cb), self);
}

/**
 * gtd_omni_area_push_message:
 * @self: a #GtdOmniArea
 * @id: an identifier for this notification
 * @text: user visible text of the notification
 * @icon: (nullable): a #GIcon
 *
 * Pushes a new message to @self.
 *
 */
void
gtd_omni_area_push_message (GtdOmniArea *self,
                            const gchar *id,
                            const gchar *text,
                            GIcon       *icon)
{
  GtkWidget *label;
  GtkWidget *box;

  g_return_if_fail (GTD_IS_OMNI_AREA (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (text != NULL);

  GTD_ENTRY;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 18);

  label = gtk_label_new (text);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_container_add (GTK_CONTAINER (box), label);

  if (icon)
    {
      GtkWidget *image = gtk_image_new_from_gicon (icon);

      gtk_widget_add_css_class (image, "dim-label");
      gtk_container_add (GTK_CONTAINER (box), image);
    }

  gtk_stack_add_named (self->status_stack, box, id);

  g_debug ("Adding message '%s' to Omni Area", id);

  g_queue_push_tail (self->messages, g_strdup (id));
  show_message (self, g_queue_get_length (self->messages) - 1);

  g_clear_handle_id (&self->switch_messages_timeout_id, g_source_remove);
  self->switch_messages_timeout_id = g_timeout_add (7500, switch_message_cb, self);

  GTD_EXIT;
}

/**
 * gtd_omni_area_withdraw_message:
 * @self: a #GtdOmniArea
 * @id: an identifier for this notification
 *
 * Withdraws a message from @self. If a message with @id doesn't
 * exist, nothing happens.
 */
void
gtd_omni_area_withdraw_message (GtdOmniArea *self,
                                const gchar *id)
{
  GtkWidget *widget;
  GList *l;

  g_return_if_fail (GTD_IS_OMNI_AREA (self));
  g_return_if_fail (id != NULL);

  GTD_ENTRY;

  widget = gtk_stack_get_child_by_name (self->status_stack, id);
  if (!widget)
    return;

  g_debug ("Removing message '%s' from Omni Area", id);

  gtk_widget_destroy (widget);

  l = g_queue_find_custom (self->messages, id, (GCompareFunc) g_strcmp0);
  g_queue_delete_link (self->messages, l);

  if (g_queue_get_length (self->messages) == 0)
    gtk_stack_set_visible_child_name (self->main_stack, "placeholder");

  GTD_EXIT;
}
