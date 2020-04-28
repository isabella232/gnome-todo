/* gtd-window.c
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

#define G_LOG_DOMAIN "GtdWindow"

#include "config.h"

#include "interfaces/gtd-activatable.h"
#include "interfaces/gtd-provider.h"
#include "interfaces/gtd-panel.h"
#include "interfaces/gtd-workspace.h"
#include "gtd-application.h"
#include "gtd-debug.h"
#include "gtd-enum-types.h"
#include "gtd-task-list-view.h"
#include "gtd-manager.h"
#include "gtd-manager-protected.h"
#include "gtd-notification.h"
#include "gtd-notification-widget.h"
#include "gtd-plugin-manager.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-window.h"
#include "gtd-window-private.h"

#include <glib/gi18n.h>

/**
 * SECTION:gtd-window
 * @short_description:main window
 * @title:GtdWindow
 * @stability:Unstable
 * @see_also:#GtdNotification
 *
 * The #GtdWindow is the main application window of GNOME To Do. Objects should
 * use this class to send notifications (see gtd_window_notify()), cancel notifications
 * (see gtd_window_cancel_notification()), change between selection and normal mode
 * and fine-tune the headerbar.
 */

struct _GtdWindow
{
  GtkApplicationWindow application;

  GtkHeaderBar       *headerbar;
  GtkStack           *stack;
  GtkWidget          *workspace_box_end;
  GtkWidget          *workspace_box_start;

  GtdNotificationWidget *notification_widget;

  GPtrArray          *workspace_header_widgets;

  GtdWorkspace       *current_workspace;
};

typedef struct
{
  GtdWindow          *window;
  gchar              *primary_text;
  gchar              *secondary_text;
} ErrorData;


G_DEFINE_TYPE (GtdWindow, gtd_window, GTK_TYPE_APPLICATION_WINDOW)



static void
setup_development_build (GtdWindow *self)
{
  GtkStyleContext *context;

  g_message (_("This is a development build of To Do. You may experience errors, wrong behaviors, "
               "and data loss."));

  context = gtk_widget_get_style_context (GTK_WIDGET (self));

  gtk_style_context_add_class (context, "devel");
}

static gboolean
is_development_build (void)
{
#ifdef DEVELOPMENT_BUILD
  return TRUE;
#else
  return FALSE;
#endif
}

static void
error_data_free (ErrorData *error_data)
{
  g_free (error_data->primary_text);
  g_free (error_data->secondary_text);
  g_free (error_data);
}

static void
error_message_notification_primary_action (GtdNotification *notification,
                                           gpointer         user_data)
{
  error_data_free (user_data);
}

static void
error_message_notification_secondary_action (GtdNotification *notification,
                                             gpointer         user_data)
{
  GtkWidget *message_dialog;
  ErrorData *data;

  data = user_data;
  message_dialog = gtk_message_dialog_new (GTK_WINDOW (data->window),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_WARNING,
                                           GTK_BUTTONS_CLOSE,
                                           "%s",
                                           data->primary_text);

  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (message_dialog),
                                            "%s",
                                            data->secondary_text);

  g_signal_connect (message_dialog,
                    "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  gtk_widget_show (message_dialog);

  error_data_free (data);
}

static void
load_geometry (GtdWindow *self)
{
  GSettings *settings;
  GtkWindow *window;
  gboolean maximized;
  gint height;
  gint width;

  window = GTK_WINDOW (self);
  settings = gtd_manager_get_settings (gtd_manager_get_default ());

  maximized = g_settings_get_boolean (settings, "window-maximized");
  g_settings_get (settings, "window-size", "(ii)", &width, &height);

  gtk_window_set_default_size (window, width, height);

  if (maximized)
    gtk_window_maximize (window);
}

static void
add_workspace (GtdWindow    *self,
               GtdWorkspace *workspace)
{
  const gchar *workspace_id;

  workspace_id = gtd_workspace_get_id (workspace);

  gtk_stack_add_named (self->stack, GTK_WIDGET (workspace), workspace_id);
}

static void
remove_all_workspace_header_widgets (GtdWindow *self)
{
  GtkWidget *parent;
  GtkWidget *widget;
  guint i;

  GTD_ENTRY;

  /* remove from the header */
  for (i = 0; i < self->workspace_header_widgets->len; i++)
    {
      widget = g_ptr_array_index (self->workspace_header_widgets, i);
      parent = gtk_widget_get_parent (widget);

      g_assert (parent == GTK_WIDGET (self->workspace_box_start) ||
                parent == GTK_WIDGET (self->workspace_box_end));
      gtk_container_remove (GTK_CONTAINER (parent), widget);
    }

  g_ptr_array_set_size (self->workspace_header_widgets, 0);

  GTD_EXIT;
}


/*
 * Callbacks
 */

static void
on_stack_visible_child_cb (GtkStack   *stack,
                           GParamSpec *pspec,
                           GtdWindow  *self)
{
  GtdWorkspace *new_workspace;

  GTD_ENTRY;

  remove_all_workspace_header_widgets (self);

  if (self->current_workspace)
    gtd_workspace_deactivate (self->current_workspace);

  new_workspace = GTD_WORKSPACE (gtk_stack_get_visible_child (stack));
  g_assert (new_workspace != NULL);

  self->current_workspace = new_workspace;
  gtd_workspace_activate (new_workspace);

  GTD_EXIT;
}

static void
on_show_error_message_cb (GtdManager                *manager,
                          const gchar               *primary_text,
                          const gchar               *secondary_text,
                          GtdNotificationActionFunc  function,
                          gpointer                   user_data,
                          GtdWindow                 *self)
{
  GtdNotification *notification;
  ErrorData *error_data;

  error_data = g_new0 (ErrorData, 1);
  notification = gtd_notification_new (primary_text, 7500);

  error_data->window = self;
  error_data->primary_text = g_strdup (primary_text);
  error_data->secondary_text = g_strdup (secondary_text);

  gtd_notification_set_primary_action (notification,
                                       error_message_notification_primary_action,
                                       error_data);

  if (!function)
    {
      gtd_notification_set_secondary_action (notification,
                                             _("Details"),
                                             error_message_notification_secondary_action,
                                             error_data);
    }
  else
    {
      gtd_notification_set_secondary_action (notification, secondary_text, function, user_data);
    }


  gtd_window_notify (self, notification);
}

static void
on_show_notification_cb (GtdManager      *manager,
                         GtdNotification *notification,
                         GtdWindow       *self)
{
  gtd_window_notify (self, notification);
}

static void
on_manager_workspace_added_cb (GtdManager   *manager,
                               GtdWorkspace *workspace,
                               GtdWindow    *self)
{
  GTD_ENTRY;

  add_workspace (self, workspace);

  GTD_EXIT;
}

static void
on_manager_workspace_removed_cb (GtdManager   *manager,
                                 GtdWorkspace *workspace,
                                 GtdWindow    *self)
{
  GTD_ENTRY;

  g_message ("Workspace removed");

  GTD_EXIT;
}


/*
 * GtkWindow overrides
 */

static void
gtd_window_unmap (GtkWidget *widget)
{
  GSettings *settings;
  GtkWindow *window;
  gboolean maximized;

  window = GTK_WINDOW (widget);
  settings = gtd_manager_get_settings (gtd_manager_get_default ());
  maximized = gtk_window_is_maximized (window);

  g_settings_set_boolean (settings, "window-maximized", maximized);

  if (!maximized)
    {
      gint height;
      gint width;

      gtk_window_get_size (window, &width, &height);
      g_settings_set (settings, "window-size", "(ii)", width, height);
    }

  GTK_WIDGET_CLASS (gtd_window_parent_class)->unmap (widget);
}

/*
 * GObject overrides
 */

static void
gtd_window_constructed (GObject *object)
{
  g_autoptr (GPtrArray) workspaces = NULL;
  GtdManager *manager;
  GtdWindow *self;
  guint i;

  self = GTD_WINDOW (object);

  G_OBJECT_CLASS (gtd_window_parent_class)->constructed (object);

  /* Load stored size */
  load_geometry (GTD_WINDOW (object));

  manager = gtd_manager_get_default ();
  g_signal_connect (manager, "show-error-message", G_CALLBACK (on_show_error_message_cb), self);
  g_signal_connect (manager, "show-notification", G_CALLBACK (on_show_notification_cb), self);

  /* Workspaces */
  workspaces = gtd_manager_get_workspaces (manager);

  for (i = 0; i < workspaces->len; i++)
    add_workspace (self, g_ptr_array_index (workspaces, i));

  g_signal_connect (manager, "workspace-added", G_CALLBACK (on_manager_workspace_added_cb), self);
  g_signal_connect (manager, "workspace-removed", G_CALLBACK (on_manager_workspace_removed_cb), self);
}

static void
gtd_window_class_init (GtdWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = gtd_window_constructed;

  widget_class->unmap = gtd_window_unmap;

  g_type_ensure (GTD_TYPE_NOTIFICATION_WIDGET);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/gtd-window.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdWindow, headerbar);
  gtk_widget_class_bind_template_child (widget_class, GtdWindow, notification_widget);
  gtk_widget_class_bind_template_child (widget_class, GtdWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, GtdWindow, workspace_box_end);
  gtk_widget_class_bind_template_child (widget_class, GtdWindow, workspace_box_start);

  gtk_widget_class_bind_template_callback (widget_class, on_stack_visible_child_cb);
}

static void
gtd_window_init (GtdWindow *self)
{
  self->workspace_header_widgets = g_ptr_array_new_with_free_func (g_object_unref);

  gtk_widget_init_template (GTK_WIDGET (self));

  /* Development build */
  if (is_development_build ())
    setup_development_build (self);
}

GtkWidget*
gtd_window_new (GtdApplication *application)
{
  return g_object_new (GTD_TYPE_WINDOW,
                       "application", application,
                       NULL);
}

/**
 * gtd_window_notify:
 * @window: a #GtdWindow
 * @notification: a #GtdNotification
 *
 * Shows a notification on the top of the main window.
 */
void
gtd_window_notify (GtdWindow       *self,
                   GtdNotification *notification)
{
  g_return_if_fail (GTD_IS_WINDOW (self));

  gtd_notification_widget_notify (self->notification_widget, notification);
}

/**
 * gtd_window_cancel_notification:
 * @window: a #GtdManager
 * @notification: a #GtdNotification
 *
 * Cancels @notification.
 */
void
gtd_window_cancel_notification (GtdWindow       *self,
                                GtdNotification *notification)
{
  g_return_if_fail (GTD_IS_WINDOW (self));

  gtd_notification_widget_cancel (self->notification_widget, notification);
}

/**
 * gtd_window_set_custom_title:
 * @window: a #GtdWindow
 * @title: (nullable): the #GtkHeaderBar title
 * @subtitle: (nullable): the #GtkHeaderBar subtitle
 *
 * Sets the #GtdWindow's headerbar title and subtitle. If @title is %NULL,
 * the header will be set to the stack switcher.
 */
void
gtd_window_set_custom_title (GtdWindow   *self,
                             const gchar *title,
                             const gchar *subtitle)
{
  g_return_if_fail (GTD_IS_WINDOW (self));

  if (title)
    {
      gtk_header_bar_set_custom_title (self->headerbar, NULL);
      gtk_header_bar_set_title (self->headerbar, title);
      gtk_header_bar_set_subtitle (self->headerbar, subtitle);
    }
  else
    {
      gtk_header_bar_set_title (self->headerbar, _("To Do"));
    }
}

/**
 * gtd_window_embed_widget_in_header:
 * @self: a #GtdWindow
 * @widget: a #GtkWidget
 * @position: either @GTK_POS_LEFT or @GTK_POS_RIGHT
 *
 * Embeds @widget into @self's header bar.
 */
void
gtd_window_embed_widget_in_header (GtdWindow       *self,
                                   GtkWidget       *widget,
                                   GtkPositionType  position)
{
  g_return_if_fail (GTD_IS_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  GTD_ENTRY;

  /* add to header */
  switch (position)
    {
    case GTK_POS_RIGHT:
      gtk_container_add (GTK_CONTAINER (self->workspace_box_end), widget);
      break;

    case GTK_POS_LEFT:
      gtk_container_add (GTK_CONTAINER (self->workspace_box_start), widget);
      break;

    case GTK_POS_TOP:
    case GTK_POS_BOTTOM:
    default:
      g_warning ("Invalid position passed");
      return;
    }

  g_ptr_array_add (self->workspace_header_widgets, g_object_ref (widget));

  GTD_EXIT;

}

/* Private functions */
void
_gtd_window_finish_startup (GtdWindow *self)
{
  g_return_if_fail (GTD_IS_WINDOW (self));
}
