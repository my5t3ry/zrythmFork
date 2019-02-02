/*
 * utils/resources.h - resource utils
 *
 * Copyright (C) 2018 Alexandros Theodotou
 *
 * This file is part of Zrythm
 *
 * Zrythm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Zrythm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Zrythm.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "utils/resources.h"

/**
 * Note: MUST be free'd
 */
static char *
get_icon_type_str (IconType icon_type)
{
  switch (icon_type)
    {
    case ICON_TYPE_ZRYTHM:
      return g_strdup ("zrythm");
    case ICON_TYPE_GNOME_BUILDER:
      return g_strdup_printf ("gnome-builder");
    }
  g_assert_not_reached ();
  return NULL;
}

GtkWidget *
resources_get_icon (IconType     icon_type,
                    const char * filename)
{
  char * icon_dir = get_icon_type_str (icon_type);
  char * path = g_strdup_printf ("%s%s%s/%s",
                   RESOURCE_PATH,
                   ICON_PATH,
                   icon_dir,
                   filename);
  g_free (icon_dir);
  GtkWidget * icon = gtk_image_new_from_resource (path);
  gtk_widget_set_visible (icon, 1);
  g_free (path);
  return icon;
}

void
resources_set_image_icon (GtkImage *   img,
                          IconType     icon_type,
                          const char * filename)
{
  char * icon_dir = get_icon_type_str (icon_type);
  char * path = g_strdup_printf ("%s%s%s/%s",
                   RESOURCE_PATH,
                   ICON_PATH,
                   icon_dir,
                   filename);
  g_free (icon_dir);
  gtk_image_set_from_resource (img, path);
  g_free (path);
}

/**
 * Sets class template from resource.
 *
 * Filename is part after .../ui/
 */
void
resources_set_class_template (GtkWidgetClass * klass,
                              const char * filename)
{
  char * path = g_strdup_printf ("%s%s%s",
                   RESOURCE_PATH,
                   TEMPLATE_PATH,
                   filename);
  gtk_widget_class_set_template_from_resource (
    klass,
    path);
  g_free (path);
}

void
resources_add_icon_to_button (GtkButton *  btn,
                              IconType     icon_type,
                              const char * path)
{
  GtkWidget * icon = resources_get_icon (icon_type,
                                         path);
  gtk_container_add (GTK_CONTAINER (btn),
                     icon);
}
