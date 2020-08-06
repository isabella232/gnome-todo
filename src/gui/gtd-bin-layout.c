/* gtd-bin-layout.c
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

#include "gtd-bin-layout.h"
#include "gtd-widget-private.h"

struct _GtdBinLayout
{
  GtkLayoutManager    parent_instance;
};

G_DEFINE_TYPE (GtdBinLayout, gtd_bin_layout, GTK_TYPE_LAYOUT_MANAGER)

static void
gtd_bin_layout_measure (GtkLayoutManager *layout_manager,
                        GtkWidget        *widget,
                        GtkOrientation    orientation,
                        int               for_size,
                        int              *minimum,
                        int              *natural,
                        int              *minimum_baseline,
                        int              *natural_baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      if (gtk_widget_should_layout (child))
        {
          int child_min = 0;
          int child_nat = 0;
          int child_min_baseline = -1;
          int child_nat_baseline = -1;

          gtk_widget_measure (child, orientation, for_size,
                              &child_min, &child_nat,
                              &child_min_baseline, &child_nat_baseline);

          *minimum = MAX (*minimum, child_min);
          *natural = MAX (*natural, child_nat);

          if (child_min_baseline > -1)
            *minimum_baseline = MAX (*minimum_baseline, child_min_baseline);
          if (child_nat_baseline > -1)
            *natural_baseline = MAX (*natural_baseline, child_nat_baseline);
        }
    }
}

static void
gtd_bin_layout_allocate (GtkLayoutManager *layout_manager,
                         GtkWidget        *widget,
                         int               width,
                         int               height,
                         int               baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child))
    {
      if (child && gtk_widget_should_layout (child))
        {
          GskTransform *transform = NULL;

          if (GTD_IS_WIDGET (child))
            {
              GtkAllocation adjusted;

              gtk_widget_get_adjusted_allocation (child, width, height, &adjusted);
              gtd_widget_update_pivot_for_geometry (GTD_WIDGET (child), &adjusted);

              transform = gtd_widget_apply_transform (GTD_WIDGET (child), NULL);
            }

          gtk_widget_allocate (child, width, height, baseline, transform);
        }
    }
}

static void
gtd_bin_layout_class_init (GtdBinLayoutClass *klass)
{
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  layout_manager_class->measure = gtd_bin_layout_measure;
  layout_manager_class->allocate = gtd_bin_layout_allocate;
}

static void
gtd_bin_layout_init (GtdBinLayout *self)
{
}

GtkLayoutManager*
gtd_bin_layout_new (void)
{
  return g_object_new (GTD_TYPE_BIN_LAYOUT, NULL);
}
