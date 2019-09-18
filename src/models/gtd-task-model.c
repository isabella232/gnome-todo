/* gtd-task-model.c
 *
 * Copyright 2018 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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

#define G_LOG_DOMAIN "GtdTaskModel"

#include "gtd-debug.h"
#include "gtd-list-store.h"
#include "gtd-manager.h"
#include "gtd-task.h"
#include "gtd-task-list.h"
#include "gtd-task-model.h"
#include "gtd-task-model-private.h"

struct _GtdTaskModel
{
  GObject             parent;

  GtkFlattenListModel *model;

  guint               number_of_tasks;

  GtdManager         *manager;
};

static void          g_list_model_iface_init                     (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GtdTaskModel, gtd_task_model, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, g_list_model_iface_init))

enum
{
  PROP_0,
  PROP_MANAGER,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * Callbacks
 */

static void
on_model_items_changed_cb (GListModel   *model,
                           guint         position,
                           guint         n_removed,
                           guint         n_added,
                           GtdTaskModel *self)
{
  GTD_TRACE_MSG ("Child model changed with position=%u, n_removed=%u, n_added=%u", position, n_removed, n_added);

  g_list_model_items_changed (G_LIST_MODEL (self), position, n_removed, n_added);
}


/*
 * GListModel iface
 */

static gpointer
gtd_task_model_get_item (GListModel *model,
                         guint       position)
{
  GtdTaskModel *self = (GtdTaskModel*) model;

  return g_list_model_get_item (G_LIST_MODEL (self->model), position);
}

static guint
gtd_task_model_get_n_items (GListModel *model)
{
  GtdTaskModel *self = (GtdTaskModel*) model;

  return g_list_model_get_n_items (G_LIST_MODEL (self->model));
}

static GType
gtd_task_model_get_item_type (GListModel *model)
{
  return GTD_TYPE_TASK;
}

static void
g_list_model_iface_init (GListModelInterface *iface)
{
  iface->get_item = gtd_task_model_get_item;
  iface->get_n_items = gtd_task_model_get_n_items;
  iface->get_item_type = gtd_task_model_get_item_type;
}


/*
 * GObject overrides
 */

static void
gtd_task_model_finalize (GObject *object)
{
  GtdTaskModel *self = (GtdTaskModel *)object;

  g_clear_object (&self->manager);
  g_clear_object (&self->model);

  G_OBJECT_CLASS (gtd_task_model_parent_class)->finalize (object);
}


static void
gtd_task_model_constructed (GObject *object)
{
  GtdTaskModel *self = (GtdTaskModel *)object;
  GListModel *model;

  g_assert (self->manager != NULL);

  model = gtd_manager_get_task_lists_model (self->manager);

  g_signal_connect_object (self->model,
                           "items-changed",
                           G_CALLBACK (on_model_items_changed_cb),
                           self,
                           0);
  gtk_flatten_list_model_set_model (self->model, model);

  G_OBJECT_CLASS (gtd_task_model_parent_class)->constructed (object);
}

static void
gtd_task_model_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GtdTaskModel *self = GTD_TASK_MODEL (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      g_value_set_object (value, self->manager);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_model_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GtdTaskModel *self = GTD_TASK_MODEL (object);

  switch (prop_id)
    {
    case PROP_MANAGER:
      g_assert (self->manager == NULL);
      self->manager = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_task_model_class_init (GtdTaskModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gtd_task_model_finalize;
  object_class->constructed = gtd_task_model_constructed;
  object_class->get_property = gtd_task_model_get_property;
  object_class->set_property = gtd_task_model_set_property;

  properties[PROP_MANAGER] = g_param_spec_object ("manager",
                                                  "Manager",
                                                  "Manager",
                                                  GTD_TYPE_MANAGER,
                                                  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gtd_task_model_init (GtdTaskModel *self)
{
  self->model = gtk_flatten_list_model_new (GTD_TYPE_OBJECT, NULL);
}

GtdTaskModel*
_gtd_task_model_new (GtdManager *manager)
{
  return g_object_new (GTD_TYPE_TASK_MODEL,
                       "manager", manager,
                       NULL);
}
