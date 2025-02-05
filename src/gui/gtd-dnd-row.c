/* gtd-dnd-row.c
 *
 * Copyright (C) 2016-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#define G_LOG_DOMAIN "GtdDndRow"

#include "gtd-dnd-row.h"
#include "gtd-manager.h"
#include "gtd-new-task-row.h"
#include "gtd-provider.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-task-row.h"

#include <math.h>

struct _GtdDndRow
{
  GtkListBoxRow       parent;

  GtkWidget          *box;
  GtkWidget          *frame;

  GtkListBoxRow      *row_above;
  gint                depth;
};

G_DEFINE_TYPE (GtdDndRow, gtd_dnd_row, GTK_TYPE_LIST_BOX_ROW)

enum {
  PROP_0,
  PROP_ROW_ABOVE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static GtdTask*
get_real_task_for_depth (GtdDndRow *self)
{
  GtdTask *task;
  gint i, task_depth;

  task = self->row_above ? gtd_task_row_get_task (GTD_TASK_ROW (self->row_above)) : NULL;
  task_depth = task ? gtd_task_get_depth (task) : -1;

  /* Find the real parent */
  for (i = task_depth - self->depth; i >= 0; i--)
    task = gtd_task_get_parent (task);

  return task;
}

static void
update_row_padding (GtdDndRow *self)
{
  gtk_widget_set_margin_start (self->box, self->depth * 32);
}

static void
on_task_updated_cb (GObject      *object,
                    GAsyncResult *result,
                    gpointer      user_data)
{
  g_autoptr (GError) error = NULL;
  GtdDndRow *self;

  self = GTD_DND_ROW (user_data);

  gtd_provider_update_task_finish (GTD_PROVIDER (object), result, &error);

  if (error)
    {
      g_warning ("Error updating task: %s", error->message);
      return;
    }

  gtk_list_box_invalidate_sort (GTK_LIST_BOX (gtk_widget_get_parent (GTK_WIDGET (self))));
}

static void
gtd_dnd_row_finalize (GObject *object)
{
  GtdDndRow *self = (GtdDndRow *)object;

  g_clear_object (&self->row_above);

  G_OBJECT_CLASS (gtd_dnd_row_parent_class)->finalize (object);
}

static void
gtd_dnd_row_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  GtdDndRow *self = GTD_DND_ROW (object);

  switch (prop_id)
    {
    case PROP_ROW_ABOVE:
      g_value_set_object (value, self->row_above);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_dnd_row_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  GtdDndRow *self = GTD_DND_ROW (object);

  switch (prop_id)
    {
    case PROP_ROW_ABOVE:
      gtd_dnd_row_set_row_above (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_dnd_row_class_init (GtdDndRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_dnd_row_finalize;
  object_class->get_property = gtd_dnd_row_get_property;
  object_class->set_property = gtd_dnd_row_set_property;

  properties[PROP_ROW_ABOVE] = g_param_spec_object ("row-above",
                                                    "Row above",
                                                    "The task row above this row",
                                                    GTD_TYPE_TASK_ROW,
                                                    G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/ui/task-list-view/gtd-dnd-row.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdDndRow, box);
  gtk_widget_class_bind_template_child (widget_class, GtdDndRow, frame);

  gtk_widget_class_set_css_name (widget_class, "dndrow");
}

static void
gtd_dnd_row_init (GtdDndRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget*
gtd_dnd_row_new (void)
{
  return g_object_new (GTD_TYPE_DND_ROW, NULL);
}

GtkListBoxRow*
gtd_dnd_row_get_row_above (GtdDndRow *self)
{
  g_return_val_if_fail (GTD_IS_DND_ROW (self), NULL);

  return self->row_above;
}

void
gtd_dnd_row_set_row_above (GtdDndRow     *self,
                           GtkListBoxRow *row)
{
  g_return_if_fail (GTD_IS_DND_ROW (self));

  if (g_set_object (&self->row_above, row))
    {
      update_row_padding (self);

      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROW_ABOVE]);
    }
}

gboolean
gtd_dnd_row_drag_motion (GtkWidget *widget,
                         gint       x,
                         gint       y)
{
  GtkAllocation alloc;
  GtdDndRow *self;

  self = GTD_DND_ROW (widget);

  gtk_widget_get_allocation (widget, &alloc);

  if (self->row_above && GTD_IS_TASK_ROW (self->row_above))
    {
      GtdTask *task;
      gint depth;

      task = gtd_task_row_get_task (GTD_TASK_ROW (self->row_above));

      if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
        depth = floor ((alloc.width - alloc.x - x) / 32);
      else
        depth = floor ((x - alloc.x) / 32);

      self->depth = CLAMP (depth, 0, gtd_task_get_depth (task) + 1);
    }
  else
    {
      self->depth = 0;
    }

  update_row_padding (self);

  return TRUE;
}

gboolean
gtd_dnd_row_drag_drop (GtkWidget  *widget,
                       GtdTaskRow *source_row,
                       GdkDrop    *drop,
                       gint        x,
                       gint        y)
{
  GtdDndRow *self;
  GtdTask *row_task, *target_task;

  self = GTD_DND_ROW (widget);

  /* Reset padding */
  update_row_padding (self);
  gtk_widget_hide (widget);

  /*
   * When the drag operation began, the source row was hidden. Now is the time
   * to show it again.
   */
  gtk_widget_show (GTK_WIDGET (source_row));

  /* Do not allow dropping on itself, nor on the new task row */
  if (!source_row || (GtkWidget*) source_row == widget || GTD_IS_NEW_TASK_ROW (source_row))
    return FALSE;

  row_task = gtd_task_row_get_task (source_row);
  target_task = get_real_task_for_depth (self);

  if (target_task)
    {
      /* Forbid adding the parent task as a subtask */
      if (gtd_task_is_subtask (row_task, target_task))
        return FALSE;

      gtd_task_add_subtask (target_task, row_task);
    }
  else
    {
      /*
       * If the user moved to depth == 0, or the first row,
       * remove the task from it's parent (if any).
       */
      if (gtd_task_get_parent (row_task))
        gtd_task_remove_subtask (gtd_task_get_parent (row_task), row_task);
    }

  /* Reset the task position */
  gtd_task_set_position (row_task, -1);

  /* Save the task */
  gtd_provider_update_task (gtd_task_get_provider (row_task),
                            row_task,
                            NULL,
                            on_task_updated_cb,
                            widget);

  return FALSE;
}
