/* gtd-utils.c
 *
 * Copyright 2018-2020 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
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
#include "gtd-max-size-layout.h"
#include "gtd-task.h"
#include "gtd-utils.h"
#include "gtd-utils-private.h"
#include "gtd-widget.h"

#include <gtk/gtk.h>

#include <string.h>


/* Combining diacritical mark?
 *  Basic range: [0x0300,0x036F]
 *  Supplement:  [0x1DC0,0x1DFF]
 *  For Symbols: [0x20D0,0x20FF]
 *  Half marks:  [0xFE20,0xFE2F]
 */
#define IS_CDM_UCS4(c) (((c) >= 0x0300 && (c) <= 0x036F)  || \
                        ((c) >= 0x1DC0 && (c) <= 0x1DFF)  || \
                        ((c) >= 0x20D0 && (c) <= 0x20FF)  || \
                        ((c) >= 0xFE20 && (c) <= 0xFE2F))

#define IS_SOFT_HYPHEN(c) ((c) == 0x00AD)


/* Copied from tracker/src/libtracker-fts/tracker-parser-glib.c under the GPL
 * And then from gnome-shell/src/shell-util.c
 *
 * Originally written by Aleksander Morgado <aleksander@gnu.org>
 */
gchar*
gtd_normalize_casefold_and_unaccent (const gchar *str)
{
  g_autofree gchar *normalized = NULL;
  gchar *tmp;
  gint i = 0;
  gint j = 0;
  gint ilen;

  if (str == NULL)
    return NULL;

  normalized = g_utf8_normalize (str, -1, G_NORMALIZE_NFKD);
  tmp = g_utf8_casefold (normalized, -1);

  ilen = strlen (tmp);

  while (i < ilen)
    {
      gunichar unichar;
      gchar *next_utf8;
      gint utf8_len;

      /* Get next character of the word as UCS4 */
      unichar = g_utf8_get_char_validated (&tmp[i], -1);

      /* Invalid UTF-8 character or end of original string. */
      if (unichar == (gunichar) -1 ||
          unichar == (gunichar) -2)
        {
          break;
        }

      /* Find next UTF-8 character */
      next_utf8 = g_utf8_next_char (&tmp[i]);
      utf8_len = next_utf8 - &tmp[i];

      if (IS_CDM_UCS4 (unichar) || IS_SOFT_HYPHEN (unichar))
        {
          /* If the given unichar is a combining diacritical mark,
           * just update the original index, not the output one */
          i += utf8_len;
          continue;
        }

      /* If already found a previous combining
       * diacritical mark, indexes are different so
       * need to copy characters. As output and input
       * buffers may overlap, need to use memmove
       * instead of memcpy */
      if (i != j)
        {
          memmove (&tmp[j], &tmp[i], utf8_len);
        }

      /* Update both indexes */
      i += utf8_len;
      j += utf8_len;
    }

  /* Force proper string end */
  tmp[j] = '\0';

  return tmp;
}

GdkContentFormats*
_gtd_get_content_formats (void)
{
  static GdkContentFormats *content_formats = NULL;

  if (!content_formats)
    content_formats = gdk_content_formats_new_for_gtype (GTD_TYPE_TASK);

  return content_formats;
}

GdkPaintable*
gtd_create_circular_paintable (GdkRGBA *color,
                               gint     size)
{
  g_autoptr (GtkSnapshot) snapshot = NULL;
  GskRoundedRect rect;

  snapshot = gtk_snapshot_new ();

  gtk_snapshot_push_rounded_clip (snapshot,
                                  gsk_rounded_rect_init_from_rect (&rect,
                                                                   &GRAPHENE_RECT_INIT (0, 0, size, size),
                                                                   size / 2.0));

  gtk_snapshot_append_color (snapshot, color, &GRAPHENE_RECT_INIT (0, 0, size, size));

  gtk_snapshot_pop (snapshot);

  return gtk_snapshot_to_paintable (snapshot, &GRAPHENE_SIZE_INIT (size, size));
}


gchar*
gtd_str_replace (const gchar *source,
                 const gchar *search,
                 const gchar *replacement)
{
  gchar *new_string, *new_aux;
  const gchar *source_aux;
  const gchar *source_aux2;
  gint64 n_ocurrences;
  gint64 replacement_len;
  gint64 search_len;
  gint64 source_len;
  gint64 final_size;
  gint64 diff;

  g_assert_nonnull (source);
  g_assert_nonnull (search);
  g_assert_nonnull (replacement);

  /* Count the number of ocurrences of "search" inside "source" */
  source_len = strlen (source);
  search_len = strlen (search);
  replacement_len = strlen (replacement);
  n_ocurrences = 0;

  for (source_aux = g_strstr_len (source, source_len, search);
       source_aux != NULL;
       source_aux = g_strstr_len (source_aux + search_len, -1, search))
    {
      n_ocurrences++;
    }

  /* Calculate size of the new string */
  diff = replacement_len - search_len;
  final_size = source_len + diff * n_ocurrences + 1;

  /* Create the new string */
  new_string = g_malloc (final_size);
  new_string[final_size - 1] = '\0';

  /*
   * And copy the contents of the source string into the new string,
   * substituting the search by replacement
   */
  source_aux2 = source;
  new_aux = new_string;

  for (source_aux = g_strstr_len (source, source_len, search);
       source_aux != NULL;
       source_aux = g_strstr_len (source_aux + search_len, -1, search))
    {
      diff = source_aux - source_aux2;

      /* Copy the non-search part between the instances of "search" in "source" */
      strncpy (new_aux, source_aux2, diff);

      new_aux += diff;

      /* Now copy the "replacement" where "search would be in source */
      strncpy (new_aux, replacement, replacement_len);

      source_aux2 = source_aux + search_len;
      new_aux += replacement_len;
    }

  /* Copy the last chunk of string if any */
  diff = source_len - (source_aux2 - source);
  strncpy (new_aux, source_aux2, diff);

  return new_string;
}

gint
gtd_collate_compare_strings (const gchar *string_a,
                             const gchar *string_b)
{
  g_autofree gchar *collated_a = NULL;
  g_autofree gchar *collated_b = NULL;

  collated_a = g_utf8_collate_key (string_a, -1);
  collated_b = g_utf8_collate_key (string_b, -1);

  return g_strcmp0 (collated_a, collated_b);
}

void
gtd_ensure_types (void)
{
  g_type_ensure (GTD_TYPE_BIN_LAYOUT);
  g_type_ensure (GTD_TYPE_MAX_SIZE_LAYOUT);
  g_type_ensure (GTD_TYPE_WIDGET);
}
