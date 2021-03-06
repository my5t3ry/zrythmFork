/*
 * gui/accelerators.h - accel
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

#include "zrythm.h"
#include "gui/accel.h"

#include <gtk/gtk.h>

/**
 * Adds all global accelerators.
 *
 * Note: not used, doesn't work.
 */
void
accel_add_all ()
{
  /*gtk_accel_map_add_entry ("<MainWindow>/File/Save",*/
                           /*GDK_KEY_s,*/
                           /*GDK_CONTROL_MASK);*/
  /*gtk_accel_map_add_entry ("<MainWindow>/File/Save As",*/
                           /*GDK_KEY_s,*/
                           /*GDK_CONTROL_MASK & GDK_SHIFT_MASK);*/
  /*gtk_accel_map_add_entry ("<MainWindow>/File/Export As",*/
                           /*GDK_KEY_e,*/
                           /*GDK_CONTROL_MASK);*/
  /*gtk_accel_map_add_entry ("<MainWindow>/File/Quit",*/
                           /*GDK_KEY_q,*/
                           /*GDK_CONTROL_MASK);*/
}

/**
 * Install accelerator for an action.
 */
void
accel_install_action_accelerator (
  const char *     primary,
  const char *     secondary,
  const char *     action_name)
{
  g_assert (ZRYTHM);
  const char *accels[] = { primary, secondary };
  gtk_application_set_accels_for_action (
    GTK_APPLICATION (ZRYTHM),
    action_name,
    accels);
}

/**
 * Install accelerator for an action.
 */
void
accel_install_primary_action_accelerator (
  const char *     primary,
  const char *     action_name)
{
  accel_install_action_accelerator (primary,
                                    NULL,
                                    action_name);
}

void
accel_set_accel_label_from_action (
  GtkAccelLabel * accel_label,
  const char *    action_name)
{
  g_assert (ZRYTHM);
  guint accel_key;
  GdkModifierType accel_mods;
  gchar ** accels =
    gtk_application_get_accels_for_action (
      GTK_APPLICATION (ZRYTHM),
      action_name);
  if (accels[0] != NULL)
    {
      gtk_accelerator_parse (
        accels[0],
        &accel_key,
        &accel_mods);
      gtk_accel_label_set_accel (
        accel_label,
        accel_key,
        accel_mods);
    }
}
