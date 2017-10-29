/* gtd-expandable-entry.h
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

#ifndef GTD_EXPANDABLE_ENTRY_H
#define GTD_EXPANDABLE_ENTRY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTD_TYPE_EXPANDABLE_ENTRY (gtd_expandable_entry_get_type())

G_DECLARE_FINAL_TYPE (GtdExpandableEntry, gtd_expandable_entry, GTD, EXPANDABLE_ENTRY, GtkEntry)

GtdExpandableEntry*  gtd_expandable_entry_new                         (void);

gboolean             gtd_expandable_entry_get_propagate_natural_width (GtdExpandableEntry *self);

void                 gtd_expandable_entry_set_propagate_natural_width (GtdExpandableEntry *self,
                                                                       gboolean            propagate_natural_width);

G_END_DECLS

#endif /* GTD_EXPANDABLE_ENTRY_H */

