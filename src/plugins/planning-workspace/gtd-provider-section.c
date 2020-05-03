/* gtd-provider-section.c
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

#include "gtd-provider-section.h"

#include "gtd-task-list-card.h"

struct _GtdProviderSection
{
  GtkFlowBoxChild     parent_instance;

  GtkLabel           *name_label;
  GtkImage           *provider_icon;
  GtkFlowBox         *task_lists_flowbox;

  GtdProvider        *provider;
};

G_DEFINE_TYPE (GtdProviderSection, gtd_provider_section, GTK_TYPE_FLOW_BOX_CHILD)

enum
{
  PROP_0,
  PROP_PROVIDER,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];


/*
 * Callbacks
 */

static gboolean
filter_by_provider_func (gpointer item,
                         gpointer user_data)
{
  GtdProviderSection *self = (GtdProviderSection *)user_data;

  return gtd_task_list_get_provider (item) == self->provider &&
         !gtd_task_list_get_archived (item);
}

static GtkWidget*
create_task_list_card_func (gpointer item,
                            gpointer user_data)
{
  return gtd_task_list_card_new (item);
}


/*
 * GObject overrides
 */

static void
gtd_provider_section_finalize (GObject *object)
{
  GtdProviderSection *self = (GtdProviderSection *)object;

  g_clear_object (&self->provider);

  G_OBJECT_CLASS (gtd_provider_section_parent_class)->finalize (object);
}

static void
gtd_provider_section_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  GtdProviderSection *self = GTD_PROVIDER_SECTION (object);

  switch (prop_id)
    {
    case PROP_PROVIDER:
      g_value_set_object (value, self->provider);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_provider_section_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  GtdProviderSection *self = GTD_PROVIDER_SECTION (object);

  switch (prop_id)
    {
    case PROP_PROVIDER:
        {
          g_autoptr (GtkFilterListModel) filter_model = NULL;
          GtdManager *manager;

          g_assert (self->provider == NULL);
          self->provider = g_value_dup_object (value);

          g_object_bind_property (self->provider,
                                  "icon",
                                  self->provider_icon,
                                  "gicon",
                                  G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

          g_object_bind_property (self->provider,
                                  "name",
                                  self->name_label,
                                  "label",
                                  G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

          manager = gtd_manager_get_default ();
          filter_model = gtk_filter_list_model_new (gtd_manager_get_task_lists_model (manager),
                                                    filter_by_provider_func,
                                                    self,
                                                    NULL);
          gtk_flow_box_bind_model (self->task_lists_flowbox,
                                   G_LIST_MODEL (filter_model),
                                   create_task_list_card_func,
                                   self,
                                   NULL);

        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gtd_provider_section_class_init (GtdProviderSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gtd_provider_section_finalize;
  object_class->get_property = gtd_provider_section_get_property;
  object_class->set_property = gtd_provider_section_set_property;

  properties[PROP_PROVIDER] = g_param_spec_object ("provider",
                                                   "Provider",
                                                   "Provider",
                                                   GTD_TYPE_PROVIDER,
                                                   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/todo/plugins/planning-workspace/gtd-provider-section.ui");

  gtk_widget_class_bind_template_child (widget_class, GtdProviderSection, name_label);
  gtk_widget_class_bind_template_child (widget_class, GtdProviderSection, provider_icon);
  gtk_widget_class_bind_template_child (widget_class, GtdProviderSection, task_lists_flowbox);

  gtk_widget_class_set_css_name (widget_class, "provider-section");
}

static void
gtd_provider_section_init (GtdProviderSection *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget*
gtd_provider_section_new (GtdProvider *provider)
{
  return g_object_new (GTD_TYPE_PROVIDER_SECTION,
                       "provider", provider,
                       NULL);
}
