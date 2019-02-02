/*
 * gui/widgets/connections.c - Manages plugins
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

#include "audio/channel.h"
#include "audio/mixer.h"
#include "plugins/plugin.h"
#include "gui/widgets/audio_unit.h"
#include "gui/widgets/audio_unit_label.h"
#include "gui/widgets/connections.h"

#include <gtk/gtk.h>

G_DEFINE_TYPE (ConnectionsWidget,
               connections_widget,
               GTK_TYPE_GRID)

static int
draw_cb (GtkWidget * widget, cairo_t * cr, void* data)
{
  guint width, height;
  GtkStyleContext *context;
  ConnectionsWidget * self = (ConnectionsWidget *) widget;
  context = gtk_widget_get_style_context (widget);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);

  AudioUnitWidget * src_au = self->src_au;
  AudioUnitWidget * dest_au = self->dest_au;
  for (int i = 0; i < src_au->num_r_labels; i++)
    {
      AudioUnitLabelWidget * src_aul = src_au->r_labels[i];
      Port * src_port = src_aul->port;
      for (int j = 0; j < dest_au->num_l_labels; j++)
        {
          AudioUnitLabelWidget * dest_aul = dest_au->l_labels[i];
          Port * dest_port = dest_aul->port;

          /*g_message ("%s %s", src_port->label, dest_port->label);*/

          if (ports_connected (src_port, dest_port))
            {
              /*GtkAllocation allocation;*/
              /*gtk_widget_get_allocation (GTK_WIDGET (track->widget->track_automation_paned),*/
                                         /*&allocation);*/

              gint wx, wy;
              gtk_widget_translate_coordinates(
                        GTK_WIDGET (self),
                        GTK_WIDGET (src_aul->drag_box),
                        0,
                        0,
                        &wx,
                        &wy);
              g_message ("src x y %d %d", wx, wy);
              cairo_move_to (cr, wx, wy);
              gtk_widget_translate_coordinates(
                        GTK_WIDGET (self),
                        GTK_WIDGET (dest_aul->drag_box),
                        0,
                        0,
                        &wx,
                        &wy);
              g_message ("dest x y %d %d", wx, wy);
              cairo_line_to (cr, wx, wy);
              cairo_stroke (cr);

            }

        }

    }

  /*Port * port = self->port;*/
  /*if ((!self->parent->is_right && self->type == AUL_TYPE_LEFT) ||*/
      /*(self->parent->is_right && self->type == AUL_TYPE_RIGHT))*/
    /*{*/
      /*cairo_set_source_rgba (cr, 0, 0, 0, 1);*/
    /*}*/
  /*else if (port->type == TYPE_AUDIO)*/
    /*{*/
      /*cairo_set_source_rgba (cr, 1, 0, 0, 1);*/
    /*}*/
  /*else if (port->type == TYPE_EVENT)*/
    /*{*/
      /*cairo_set_source_rgba (cr, 0, 0, 1, 1);*/
    /*}*/
  /*else if (port->type == TYPE_CONTROL)*/
    /*{*/
      /*cairo_set_source_rgba (cr, 0, 1, 0, 1);*/
    /*}*/
  /*else*/
    /*{*/
      /*cairo_set_source_rgba (cr, 0.5, 0.5, 0.5, 1);*/
    /*}*/
  /*draw_text (cr, self->port->label);*/

  return FALSE;
}

static void
on_selector_changed (GtkComboBox * widget,
               gpointer     user_data)
{
  ConnectionsWidget * self = Z_CONNECTIONS_WIDGET (user_data);

  GtkTreeIter iter;
  gtk_combo_box_get_active_iter (widget, &iter);
  GtkTreeModel * model = gtk_combo_box_get_model (widget);
  GValue value1 = G_VALUE_INIT;
  gtk_tree_model_get_value (model, &iter, 1, &value1);
  AUWidgetType type = g_value_get_int (&value1);

  switch (type)
    {
    case AUWT_NONE:
      break;
    case AUWT_CHANNEL:
        {
          GValue value = G_VALUE_INIT;
          gtk_tree_model_get_value (model, &iter, 2, &value);
          Channel * chan = g_value_get_pointer (&value);
          if (GTK_COMBO_BOX (self->select_src_cb) == widget)
            {
              audio_unit_widget_set_from_channel (self->src_au,
                                                  chan);
            }
          else
            {
              audio_unit_widget_set_from_channel (self->dest_au,
                                                  chan);
            }
        }
      break;
    case AUWT_MASTER:
      break;
    case AUWT_PLUGIN:
        {
          GValue value = G_VALUE_INIT;
          gtk_tree_model_get_value (model, &iter, 2, &value);
          Plugin * plugin = g_value_get_pointer (&value);
          if (GTK_COMBO_BOX (self->select_src_cb) == widget)
            {
              audio_unit_widget_set_from_plugin (self->src_au,
                                                  plugin);
            }
          else
            {
              audio_unit_widget_set_from_plugin (self->dest_au,
                                                  plugin);
            }
        }
      break;
    case AUWT_RACK_CONTROLLER:
      break;
    case AUWT_ENGINE:
      break;
    }
}

static GtkTreeModel *
create_audio_units_store ()
{
  GtkTreeIter iter, iter2, iter3;
  GtkTreeStore *store;

  store = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_POINTER);

  /* add engine */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      0, "Audio Engine",
                      1, AUWT_ENGINE,
                      2, NULL,
                      -1);

  /* add channels */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      0, "Channels",
                      1, AUWT_NONE,
                      2, NULL,
                      -1);

  FOREACH_CH
    {
      Channel * channel = MIXER->channels[i];
      char * label = g_strdup_printf ("%d:%s", i, channel->name);
      gtk_tree_store_append (store, &iter2, &iter);
      gtk_tree_store_set (store, &iter2,
                          0, label,
                          1, AUWT_NONE,
                          2, NULL,
                          -1);
      g_free (label);
      label = g_strdup_printf ("%s Channel", channel->name);
      gtk_tree_store_append (store, &iter3, &iter2);
      gtk_tree_store_set (store, &iter3,
                          0, label,
                          1, AUWT_CHANNEL,
                          2, channel,
                          -1);
      g_free (label);
      for (int j = 0; j < STRIP_SIZE; j++)
        {
          Plugin * plugin = channel->strip[j];
          if (plugin)
            {
              char * label = g_strdup_printf ("%s:%d:%s",
                                              channel->name,
                                              j,
                                              plugin->descr->name);
              gtk_tree_store_append (store, &iter3, &iter2);
              gtk_tree_store_set (store, &iter3,
                                  0, label,
                                  1, AUWT_PLUGIN,
                                  2, plugin,
                                  -1);
              g_free (label);
            }
        }
    }
  gtk_tree_store_append (store, &iter2, &iter);
  gtk_tree_store_set (store, &iter2,
                      0, MIXER->master->name,
                      1, AUWT_NONE,
                      2, NULL,
                      -1);
  gtk_tree_store_append (store, &iter3, &iter2);
  gtk_tree_store_set (store, &iter3,
                      0, "Master Channel",
                      1, AUWT_MASTER,
                      2, MIXER->master,
                      -1);
  for (int j = 0; j < STRIP_SIZE; j++)
    {
      Plugin * plugin = MIXER->master->strip[j];
      if (plugin)
        {
          char * label = g_strdup_printf ("%d:%s", j, plugin->descr->name);
          gtk_tree_store_append (store, &iter3, &iter2);
          gtk_tree_store_set (store, &iter3,
                              0, label,
                              1, AUWT_PLUGIN,
                              2, plugin,
                              -1);
          g_free (label);
        }
    }

  /* add rack controllers */
  gtk_tree_store_append (store, &iter, NULL);
  gtk_tree_store_set (store, &iter,
                      0, "Rack Controllers",
                      1, AUWT_NONE,
                      2, NULL,
                      -1);

  return GTK_TREE_MODEL (store);
}

static void
setup_combo_box (GtkComboBoxText * cb)
{
  GtkTreeModel * model = create_audio_units_store ();
  gtk_combo_box_set_model (GTK_COMBO_BOX (cb),
                           model);
  gtk_cell_layout_clear (GTK_CELL_LAYOUT (cb));
  GtkCellRenderer* renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (cb),
                              renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (cb),
                                  renderer,
                                  "text", 0,
                                  NULL);

  GtkTreeIter iter;
  GtkTreePath * path = gtk_tree_path_new_from_indices (0, -1);
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_path_free (path);
  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (cb),
                                 &iter);
}

void
connections_widget_refresh (ConnectionsWidget * self)
{
  setup_combo_box (self->select_src_cb);
  setup_combo_box (self->select_dest_cb);
}


static void
connections_widget_class_init (ConnectionsWidgetClass * klass)
{
  gtk_widget_class_set_template_from_resource (
                        GTK_WIDGET_CLASS (klass),
                        "/org/zrythm/ui/connections.ui");

  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        ConnectionsWidget,
                                        select_src_cb);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        ConnectionsWidget,
                                        select_dest_cb);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        ConnectionsWidget,
                                        src_au_viewport);
  gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass),
                                        ConnectionsWidget,
                                        dest_au_viewport);
}

static void
connections_widget_init (ConnectionsWidget * self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->src_au = audio_unit_widget_new (0);
  gtk_container_add (GTK_CONTAINER (self->src_au_viewport),
                     GTK_WIDGET (self->src_au));
  self->dest_au = audio_unit_widget_new (1);
  gtk_container_add (GTK_CONTAINER (self->dest_au_viewport),
                     GTK_WIDGET (self->dest_au));

  /* connect signals */
  g_signal_connect (G_OBJECT (self->select_src_cb), "changed",
                    G_CALLBACK (on_selector_changed), self);
  g_signal_connect (G_OBJECT (self->select_dest_cb), "changed",
                    G_CALLBACK (on_selector_changed), self);
  g_signal_connect (G_OBJECT (self), "draw",
                    G_CALLBACK (draw_cb), self);
}
