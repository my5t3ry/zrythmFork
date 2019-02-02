/*
 * plugins/lv2_gtk.h - The front end of LV2 UIs
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

#include <math.h>

#include <gtk/gtk.h>

#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/port-props/port-props.h"

#include "zrythm.h"
#include "plugins/lv2_plugin.h"
#include "plugins/plugin_manager.h"

#if GTK_MAJOR_VERSION == 3
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif


bool no_menu = false;
bool generic_ui = false; /* FIXME */

/** Widget for a control. */
typedef struct {
	GtkSpinButton* spin;
	GtkWidget*     control;
} Controller;

static float
get_float(const LilvNode* node, float fallback)
{
	if (lilv_node_is_float(node) || lilv_node_is_int(node)) {
		return lilv_node_as_float(node);
	}

	return fallback;
}

static GtkWidget*
new_box(gboolean horizontal, gint spacing)
{
	#if GTK_MAJOR_VERSION == 3
	return gtk_box_new(
		horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL,
		spacing);
	#else
	return (horizontal
	        ? gtk_hbox_new(FALSE, spacing)
	        : gtk_vbox_new(FALSE, spacing));
	#endif
}

static GtkWidget*
new_hscale(gdouble min, gdouble max, gdouble step)
{
	#if GTK_MAJOR_VERSION == 3
	return gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, step);
	#else
	return gtk_hscale_new_with_range(min, max, step);
	#endif
}

static void
size_request(GtkWidget* widget, GtkRequisition* req)
{
	#if GTK_MAJOR_VERSION == 3
	gtk_widget_get_preferred_size(widget, NULL, req);
	#else
	gtk_widget_size_request(widget, req);
	#endif
}

static void
on_window_destroy(GtkWidget* widget,
                  gpointer   data)
{
  Lv2Plugin * plugin = (Lv2Plugin *) data;
  plugin->window = NULL;
}

const char*
lv2_native_ui_type(Lv2Plugin* plugin)
{
#if GTK_MAJOR_VERSION == 2
	return "http://lv2plug.in/ns/extensions/ui#GtkUI";
#elif GTK_MAJOR_VERSION == 3
	return "http://lv2plug.in/ns/extensions/ui#Gtk3UI";
#else
	return NULL;
#endif
}

static void
on_save_activate(GtkWidget* widget, void* ptr)
{
	Lv2Plugin* plugin = (Lv2Plugin*)ptr;
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
		"Save State",
		(GtkWindow*)plugin->window,
		GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Save", GTK_RESPONSE_ACCEPT,
		NULL);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char* path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		char* base = g_build_filename(path, "/", NULL);
		lv2_save(plugin, base);
		g_free(path);
		g_free(base);
	}

	gtk_widget_destroy(dialog);
}

static void
on_quit_activate(GtkWidget* widget, gpointer data)
{
	GtkWidget* window = (GtkWidget*)data;
	gtk_widget_destroy(window);
}

typedef struct {
	Lv2Plugin*     plugin;
	LilvNode* preset;
} PresetRecord;

static char*
symbolify(const char* in)
{
	const size_t len = strlen(in);
	char*        out = (char*)calloc(len + 1, 1);
	for (size_t i = 0; i < len; ++i) {
		if (g_ascii_isalnum(in[i])) {
			out[i] = in[i];
		} else {
			out[i] = '_';
		}
	}
	return out;
}

static void
set_window_title(Lv2Plugin* plugin)
{
	LilvNode*   name   = lilv_plugin_get_name(plugin->lilv_plugin);
	const char* name_str = lilv_node_as_string(name);
	if (plugin->preset) {
		const char* preset_label = lilv_state_get_label(plugin->preset);
		char* title = g_strdup_printf("%s - %s", name_str, preset_label);
		gtk_window_set_title(GTK_WINDOW(plugin->window), title);
		free(title);
	} else {
		gtk_window_set_title(GTK_WINDOW(plugin->window), name_str);
	}
	lilv_node_free(name);
}

static void
on_preset_activate(GtkWidget* widget, gpointer data)
{
  PresetRecord* record = (PresetRecord*)data;
  if (GTK_CHECK_MENU_ITEM(widget) != record->plugin->active_preset_item) {
          lv2_apply_preset(record->plugin, record->preset);
          if (record->plugin->active_preset_item) {
                  gtk_check_menu_item_set_active(record->plugin->active_preset_item, FALSE);
          }

          record->plugin->active_preset_item = GTK_CHECK_MENU_ITEM(widget);
          gtk_check_menu_item_set_active(record->plugin->active_preset_item, TRUE);
          set_window_title(record->plugin);
  }
}

static void
on_preset_destroy(gpointer data, GClosure* closure)
{
	PresetRecord* record = (PresetRecord*)data;
	lilv_node_free(record->preset);
	free(record);
}

typedef struct {
	GtkMenuItem* item;
	char*        label;
	GtkMenu*     menu;
	GSequence*   banks;
} PresetMenu;

static PresetMenu*
pset_menu_new(const char* label)
{
	PresetMenu* menu = (PresetMenu*)calloc (1, sizeof(PresetMenu));
	menu->label = g_strdup(label);
	menu->item  = GTK_MENU_ITEM(gtk_menu_item_new_with_label(menu->label));
	menu->menu  = GTK_MENU(gtk_menu_new());
	menu->banks = NULL;
	return menu;
}

static void
pset_menu_free(PresetMenu* menu)
{
	if (menu->banks) {
		for (GSequenceIter* i = g_sequence_get_begin_iter(menu->banks);
		     !g_sequence_iter_is_end(i);
		     i = g_sequence_iter_next(i)) {
			PresetMenu* bank_menu = (PresetMenu*)g_sequence_get(i);
			pset_menu_free(bank_menu);
		}
		g_sequence_free(menu->banks);
	}

	free(menu->label);
	free(menu);
}

static gint
menu_cmp(gconstpointer a, gconstpointer b, gpointer data)
{
	return strcmp(((PresetMenu*)a)->label, ((PresetMenu*)b)->label);
}

static PresetMenu*
get_bank_menu(Lv2Plugin* plugin, PresetMenu* menu, const LilvNode* bank)
{
	LilvNode* label = lilv_world_get(
		LILV_WORLD, bank, plugin->nodes.rdfs_label, NULL);

	const char*    uri = lilv_node_as_string(bank);
	const char*    str = label ? lilv_node_as_string(label) : uri;
	PresetMenu     key = { NULL, (char*)str, NULL, NULL };
	GSequenceIter* i   = g_sequence_lookup(menu->banks, &key, menu_cmp, NULL);
	if (!i) {
		PresetMenu* bank_menu = pset_menu_new(str);
		gtk_menu_item_set_submenu(bank_menu->item, GTK_WIDGET(bank_menu->menu));
		g_sequence_insert_sorted(menu->banks, bank_menu, menu_cmp, NULL);
		return bank_menu;
	}
	return (PresetMenu*)g_sequence_get(i);
}

static int
add_preset_to_menu(Lv2Plugin*           plugin,
                   const LilvNode* node,
                   const LilvNode* title,
                   void*           data)
{
	PresetMenu* menu  = (PresetMenu*)data;
	const char* label = lilv_node_as_string(title);
	GtkWidget*  item  = gtk_check_menu_item_new_with_label(label);
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
	if (plugin->preset &&
	    lilv_node_equals(lilv_state_get_uri(plugin->preset), node)) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
		plugin->active_preset_item = GTK_CHECK_MENU_ITEM(item);
	}

	LilvNode* bank = lilv_world_get(
		LILV_WORLD, node, plugin->nodes.pset_bank, NULL);

	if (bank) {
		PresetMenu* bank_menu = get_bank_menu(plugin, menu, bank);
		gtk_menu_shell_append(GTK_MENU_SHELL(bank_menu->menu), item);
	} else {
		gtk_menu_shell_append(GTK_MENU_SHELL(menu->menu), item);
	}

	PresetRecord* record = (PresetRecord*)calloc(1, sizeof(PresetRecord));
	record->plugin   = plugin;
	record->preset = lilv_node_duplicate(node);

	g_signal_connect_data(G_OBJECT(item), "activate",
	                      G_CALLBACK(on_preset_activate),
	                      record, on_preset_destroy,
	                      (GConnectFlags)0);

	return 0;
}

static void
finish_menu(PresetMenu* menu)
{
	for (GSequenceIter* i = g_sequence_get_begin_iter(menu->banks);
	     !g_sequence_iter_is_end(i);
	     i = g_sequence_iter_next(i)) {
		PresetMenu* bank_menu = (PresetMenu*)g_sequence_get(i);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu->menu),
		                      GTK_WIDGET(bank_menu->item));
	}
	g_sequence_free(menu->banks);
}

static void
rebuild_preset_menu(Lv2Plugin* plugin, GtkContainer* pset_menu)
{
	// Clear current menu
	plugin->active_preset_item = NULL;
	for (GList* items = g_list_nth(gtk_container_get_children(pset_menu), 3);
	     items;
	     items = items->next) {
		gtk_container_remove(pset_menu, GTK_WIDGET(items->data));
	}

	// Load presets and build new menu
	PresetMenu menu = {
		NULL, NULL, GTK_MENU(pset_menu),
		g_sequence_new((GDestroyNotify)pset_menu_free)
	};
	lv2_load_presets(plugin, add_preset_to_menu, &menu);
	finish_menu(&menu);
	gtk_widget_show_all(GTK_WIDGET(pset_menu));
}

static void
on_save_preset_activate(GtkWidget* widget, void* ptr)
{
	Lv2Plugin* plugin = (Lv2Plugin*)ptr;

	GtkWidget* dialog = gtk_file_chooser_dialog_new(
		"Save Preset",
		(GtkWindow*)plugin->window,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		"_Cancel", GTK_RESPONSE_REJECT,
		"_Save", GTK_RESPONSE_ACCEPT,
		NULL);

	char* dot_lv2 = g_build_filename(g_get_home_dir(), ".lv2", NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), dot_lv2);
	free(dot_lv2);

	GtkWidget* content    = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkBox*    box        = GTK_BOX(new_box(true, 8));
	GtkWidget* uri_label  = gtk_label_new("URI (Optional):");
	GtkWidget* uri_entry  = gtk_entry_new();
	GtkWidget* add_prefix = gtk_check_button_new_with_mnemonic(
		"_Prefix plugin name");

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(add_prefix), TRUE);
	gtk_box_pack_start(box, uri_label, FALSE, TRUE, 2);
	gtk_box_pack_start(box, uri_entry, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(content), GTK_WIDGET(box), FALSE, FALSE, 6);
	gtk_box_pack_start(GTK_BOX(content), add_prefix, FALSE, FALSE, 6);

	gtk_widget_show_all(GTK_WIDGET(dialog));
	gtk_entry_set_activates_default(GTK_ENTRY(uri_entry), TRUE);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		LilvNode*   plug_name = lilv_plugin_get_name(plugin->lilv_plugin);
		const char* path      = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		const char* uri       = gtk_entry_get_text(GTK_ENTRY(uri_entry));
		const char* prefix    = "";
		const char* sep       = "";
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(add_prefix))) {
			prefix = lilv_node_as_string(plug_name);
			sep    = "_";
		}

		char* dirname  = g_path_get_dirname(path);
		char* basename = g_path_get_basename(path);
		char* sym      = symbolify(basename);
		char* sprefix  = symbolify(prefix);
		char* bundle   = g_strjoin(NULL, sprefix, sep, sym, ".preset.lv2/", NULL);
		char* file     = g_strjoin(NULL, sym, ".ttl", NULL);
		char* dir      = g_build_filename(dirname, bundle, NULL);

		lv2_save_preset(plugin, dir, (strlen(uri) ? uri : NULL), basename, file);

		// Reload bundle into the world
		LilvNode* ldir = lilv_new_file_uri(LILV_WORLD, NULL, dir);
		lilv_world_unload_bundle(LILV_WORLD, ldir);
		lilv_world_load_bundle(LILV_WORLD, ldir);
		lilv_node_free(ldir);

		// Rebuild preset menu and update window title
		rebuild_preset_menu(plugin, GTK_CONTAINER(gtk_widget_get_parent(widget)));
		set_window_title(plugin);

		g_free(dir);
		g_free(file);
		g_free(bundle);
		free(sprefix);
		free(sym);
		g_free(basename);
		g_free(dirname);
		lilv_node_free(plug_name);
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
on_delete_preset_activate(GtkWidget* widget, void* ptr)
{
	Lv2Plugin* plugin = (Lv2Plugin*)ptr;
	if (!plugin->preset) {
		return;
	}

	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		"Delete Preset?",
		(GtkWindow*)plugin->window,
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		"_Cancel", GTK_RESPONSE_REJECT,
		"_OK", GTK_RESPONSE_ACCEPT,
		NULL);

	char* msg = g_strdup_printf("Delete preset \"%s\" from the file system?",
	                            lilv_state_get_label(plugin->preset));

	GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget* text    = gtk_label_new(msg);
	gtk_box_pack_start(GTK_BOX(content), text, TRUE, TRUE, 4);

	gtk_widget_show_all(dialog);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		lv2_delete_current_preset(plugin);
		rebuild_preset_menu(plugin, GTK_CONTAINER(gtk_widget_get_parent(widget)));
	}

	lilv_state_free(plugin->preset);
	plugin->preset = NULL;
	set_window_title(plugin);

	g_free(msg);
	gtk_widget_destroy(text);
	gtk_widget_destroy(dialog);
}

static void
set_control(const Lv2ControlID* control,
            uint32_t         size,
            LV2_URID         type,
            const void*      body)
{
  if (!control->plugin->updating)
    {
      lv2_set_control(control, size, type, body);
    }
}

static bool
differ_enough(float a, float b)
{
	return fabsf(a - b) >= FLT_EPSILON;
}

void
lv2_gtk_set_float_control(const Lv2ControlID* control, float value)
{
  if (control->value_type == control->plugin->forge.Int) {
          const int32_t ival = lrint(value);
          set_control(control, sizeof(ival), control->plugin->forge.Int, &ival);
  } else if (control->value_type == control->plugin->forge.Long) {
          const int64_t lval = lrint(value);
          set_control(control, sizeof(lval), control->plugin->forge.Long, &lval);
  } else if (control->value_type == control->plugin->forge.Float) {
          set_control(control, sizeof(value), control->plugin->forge.Float, &value);
  } else if (control->value_type == control->plugin->forge.Double) {
          const double dval = value;
          set_control(control, sizeof(dval), control->plugin->forge.Double, &dval);
  } else if (control->value_type == control->plugin->forge.Bool) {
          const int32_t ival = value;
          set_control(control, sizeof(ival), control->plugin->forge.Bool, &ival);
  }
  else
    {
      /* FIXME ? */
      set_control(control, sizeof(value), control->plugin->forge.Float, &value);
    }

  Controller* controller = (Controller*)control->widget;
  if (controller && controller->spin &&
      differ_enough(gtk_spin_button_get_value(controller->spin), value)) {
          gtk_spin_button_set_value(controller->spin, value);
  }
}

static double
get_atom_double(Lv2Plugin* plugin, uint32_t size, LV2_URID type, const void* body)
{
	if (type == plugin->forge.Int || type == plugin->forge.Bool) {
		return *(const int32_t*)body;
	} else if (type == plugin->forge.Long) {
		return *(const int64_t*)body;
	} else if (type == plugin->forge.Float) {
		return *(const float*)body;
	} else if (type == plugin->forge.Double) {
		return *(const double*)body;
	}
	return NAN;
}

static void
control_changed(Lv2Plugin*       plugin,
                Controller* controller,
                uint32_t    size,
                LV2_URID    type,
                const void* body)
{
	GtkWidget*   widget = controller->control;
	const double fvalue = get_atom_double(plugin, size, type, body);

	if (!isnan(fvalue)) {
		if (GTK_IS_COMBO_BOX(widget)) {
			GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
			GValue        value = { 0, { { 0 } } };
			GtkTreeIter   i;
			bool          valid = gtk_tree_model_get_iter_first(model, &i);
			while (valid) {
				gtk_tree_model_get_value(model, &i, 0, &value);
				const double v = g_value_get_float(&value);
				g_value_unset(&value);
				if (fabs(v - fvalue) < FLT_EPSILON) {
					gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget), &i);
					return;
				}
				valid = gtk_tree_model_iter_next(model, &i);
			}
		} else if (GTK_IS_TOGGLE_BUTTON(widget)) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
			                             fvalue > 0.0f);
		} else if (GTK_IS_RANGE(widget)) {
			gtk_range_set_value(GTK_RANGE(widget), fvalue);
		} else {
			fprintf(stderr, "Unknown widget type for value\n");
		}

		if (controller->spin) {
			// Update spinner for numeric control
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(controller->spin),
			                          fvalue);
		}
	} else if (GTK_IS_ENTRY(widget) && type == plugin->urids.atom_String) {
		gtk_entry_set_text(GTK_ENTRY(widget), (const char*)body);
	} else if (GTK_IS_FILE_CHOOSER(widget) && type == plugin->urids.atom_Path) {
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(widget), (const char*)body);
	} else {
		fprintf(stderr, "Unknown widget type for value\n");
	}
}

static int
patch_set_get(Lv2Plugin*                  plugin,
              const LV2_Atom_Object* obj,
              const LV2_Atom_URID**  property,
              const LV2_Atom**       value)
{
	lv2_atom_object_get(obj,
	                    plugin->urids.patch_property, (const LV2_Atom*)property,
	                    plugin->urids.patch_value,    value,
	                    0);
	if (!*property) {
		fprintf(stderr, "patch:Set message with no property\n");
		return 1;
	} else if ((*property)->atom.type != plugin->forge.URID) {
		fprintf(stderr, "patch:Set property is not a URID\n");
		return 1;
	}

	return 0;
}

static int
patch_put_get(Lv2Plugin*                   plugin,
              const LV2_Atom_Object*  obj,
              const LV2_Atom_Object** body)
{
	lv2_atom_object_get(obj,
	                    plugin->urids.patch_body, (const LV2_Atom*)body,
	                    0);
	if (!*body) {
		fprintf(stderr, "patch:Put message with no body\n");
		return 1;
	} else if (!lv2_atom_forge_is_object_type(&plugin->forge, (*body)->atom.type)) {
		fprintf(stderr, "patch:Put body is not an object\n");
		return 1;
	}

	return 0;
}

static void
property_changed(Lv2Plugin* plugin, LV2_URID key, const LV2_Atom* value)
{
    Lv2ControlID* control = lv2_get_property_control(&plugin->controls, key);
    if (control) {
            control_changed(plugin,
                            (Controller*)control->widget,
                            value->size,
                            value->type,
                            value + 1);
    }
}

void
lv2_ui_port_event(Lv2Plugin*       plugin,
                   uint32_t    port_index,
                   uint32_t    buffer_size,
                   uint32_t    protocol,
                   const void* buffer)
{
	if (plugin->ui_instance) {
		suil_instance_port_event(plugin->ui_instance, port_index,
		                         buffer_size, protocol, buffer);
		return;
	} else if (protocol == 0 && (Controller*)plugin->ports[port_index].widget) {
		control_changed(plugin,
		                (Controller*)plugin->ports[port_index].widget,
		                buffer_size,
		                plugin->forge.Float,
		                buffer);
		return;
	} else if (protocol == 0) {
		return;  // No widget (probably notOnGUI)
	} else if (protocol != plugin->urids.atom_eventTransfer) {
		fprintf(stderr, "Unknown port event protocol\n");
		return;
	}

	const LV2_Atom* atom = (const LV2_Atom*)buffer;
	if (lv2_atom_forge_is_object_type(&plugin->forge, atom->type)) {
		plugin->updating = true;
		const LV2_Atom_Object* obj = (const LV2_Atom_Object*)buffer;
		if (obj->body.otype == plugin->urids.patch_Set) {
			const LV2_Atom_URID* property = NULL;
			const LV2_Atom*      value    = NULL;
			if (!patch_set_get(plugin, obj, &property, &value)) {
				property_changed(plugin, property->body, value);
			}
		} else if (obj->body.otype == plugin->urids.patch_Put) {
			const LV2_Atom_Object* body = NULL;
			if (!patch_put_get(plugin, obj, &body)) {
				LV2_ATOM_OBJECT_FOREACH(body, prop) {
					property_changed(plugin, prop->key, &prop->value);
				}
			}
		} else {
			printf("Unknown object type?\n");
		}
		plugin->updating = false;
	}
}

static gboolean
scale_changed(GtkRange* range, gpointer data)
{
	lv2_gtk_set_float_control((const Lv2ControlID*)data, gtk_range_get_value(range));
	return FALSE;
}

static gboolean
spin_changed(GtkSpinButton* spin, gpointer data)
{
	const Lv2ControlID* control    = (const Lv2ControlID*)data;
	Controller*      controller = (Controller*)control->widget;
	GtkRange*        range      = GTK_RANGE(controller->control);
	const double     value      = gtk_spin_button_get_value(spin);
	if (differ_enough(gtk_range_get_value(range), value)) {
		gtk_range_set_value(range, value);
	}
	return FALSE;
}

static gboolean
log_scale_changed(GtkRange* range, gpointer data)
{
	lv2_gtk_set_float_control((const Lv2ControlID*)data, expf(gtk_range_get_value(range)));
	return FALSE;
}

static gboolean
log_spin_changed(GtkSpinButton* spin, gpointer data)
{
	const Lv2ControlID* control    = (const Lv2ControlID*)data;
	Controller*      controller = (Controller*)control->widget;
	GtkRange*        range      = GTK_RANGE(controller->control);
	const double     value      = gtk_spin_button_get_value(spin);
	if (differ_enough(gtk_range_get_value(range), logf(value))) {
		gtk_range_set_value(range, logf(value));
	}
	return FALSE;
}

static void
combo_changed(GtkComboBox* box, gpointer data)
{
	const Lv2ControlID* control = (const Lv2ControlID*)data;

	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter(box, &iter)) {
		GtkTreeModel* model = gtk_combo_box_get_model(box);
		GValue        value = { 0, { { 0 } } };

		gtk_tree_model_get_value(model, &iter, 0, &value);
		const double v = g_value_get_float(&value);
		g_value_unset(&value);

		lv2_gtk_set_float_control(control, v);
	}
}

static gboolean
toggle_changed(GtkToggleButton* button, gpointer data)
{
	lv2_gtk_set_float_control((const Lv2ControlID*)data,
	                  gtk_toggle_button_get_active(button) ? 1.0f : 0.0f);
	return FALSE;
}

static void
string_changed(GtkEntry* widget, gpointer data)
{
	Lv2ControlID*  control = (Lv2ControlID*)data;
	const char* string  = gtk_entry_get_text(widget);

	set_control(control, strlen(string) + 1, control->plugin->forge.String, string);
}

static void
file_changed(GtkFileChooserButton* widget,
             gpointer              data)
{
	Lv2ControlID*  control   = (Lv2ControlID*)data;
	Lv2Plugin*       plugin     = control->plugin;
	const char* filename = gtk_file_chooser_get_filename(
		GTK_FILE_CHOOSER(widget));

	set_control(control, strlen(filename), plugin->forge.Path, filename);
}

static Controller*
new_controller(GtkSpinButton* spin, GtkWidget* control)
{
	Controller* controller = (Controller*)calloc(1, sizeof(Controller));
	controller->spin    = spin;
	controller->control = control;
	return controller;
}

static Controller*
make_combo(Lv2ControlID* record, float value)
{
	GtkListStore* list_store = gtk_list_store_new(
		2, G_TYPE_FLOAT, G_TYPE_STRING);
	int active = -1;
	for (size_t i = 0; i < record->n_points; ++i) {
		const Lv2ScalePoint* point = &record->points[i];
		GtkTreeIter       iter;
		gtk_list_store_append(list_store, &iter);
		gtk_list_store_set(list_store, &iter,
		                   0, point->value,
		                   1, point->label,
		                   -1);
		if (fabs(value - point->value) < FLT_EPSILON) {
			active = i;
		}
	}

	GtkWidget* combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(list_store));
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
	g_object_unref(list_store);

	gtk_widget_set_sensitive(combo, record->is_writable);

	GtkCellRenderer* cell = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), cell, "text", 1, NULL);

	if (record->is_writable) {
		g_signal_connect(G_OBJECT(combo), "changed",
		                 G_CALLBACK(combo_changed), record);
	}

	return new_controller(NULL, combo);
}

static Controller*
make_log_slider(Lv2ControlID* record, float value)
{
	const float min   = get_float(record->min, 0.0f);
	const float max   = get_float(record->max, 1.0f);
	const float lmin  = logf(min);
	const float lmax  = logf(max);
	const float ldft  = logf(value);
	GtkWidget*  scale = new_hscale(lmin, lmax, 0.001);
	GtkWidget*  spin  = gtk_spin_button_new_with_range(min, max, 0.000001);

	gtk_widget_set_sensitive(scale, record->is_writable);
	gtk_widget_set_sensitive(spin, record->is_writable);

	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	gtk_range_set_value(GTK_RANGE(scale), ldft);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), value);

	if (record->is_writable) {
		g_signal_connect(G_OBJECT(scale), "value-changed",
		                 G_CALLBACK(log_scale_changed), record);
		g_signal_connect(G_OBJECT(spin), "value-changed",
		                 G_CALLBACK(log_spin_changed), record);
	}

	return new_controller(GTK_SPIN_BUTTON(spin), scale);
}

static Controller*
make_slider(Lv2ControlID* record, float value)
{
  const float  min   = get_float(record->min, 0.0f);
  const float  max   = get_float(record->max, 1.0f);
  const double step  = record->is_integer ? 1.0 : ((max - min) / 100.0);
  GtkWidget*   scale = new_hscale(min, max, step);
  GtkWidget*   spin  = gtk_spin_button_new_with_range(min, max, step);

  gtk_widget_set_sensitive(scale, record->is_writable);
  gtk_widget_set_sensitive(spin, record->is_writable);

  if (record->is_integer) {
          gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 0);
  } else {
          gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 7);
  }

  gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
  gtk_range_set_value(GTK_RANGE(scale), value);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), value);
  if (record->points) {
          for (size_t i = 0; i < record->n_points; ++i) {
                  const Lv2ScalePoint* point = &record->points[i];

                  gchar* str = g_markup_printf_escaped(
                          "<span font_size=\"small\">%s</span>", point->label);
                  gtk_scale_add_mark(
                          GTK_SCALE(scale), point->value, GTK_POS_TOP, str);
          }
  }

  if (record->is_writable) {
          g_signal_connect(G_OBJECT(scale), "value-changed",
                           G_CALLBACK(scale_changed), record);
          g_signal_connect(G_OBJECT(spin), "value-changed",
                           G_CALLBACK(spin_changed), record);
  }

  return new_controller(GTK_SPIN_BUTTON(spin), scale);
}

static Controller*
make_toggle(Lv2ControlID* record, float value)
{
	GtkWidget* check = gtk_check_button_new();

	gtk_widget_set_sensitive(check, record->is_writable);

	if (value) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
	}

	if (record->is_writable) {
		g_signal_connect(G_OBJECT(check), "toggled",
		                 G_CALLBACK(toggle_changed), record);
	}

	return new_controller(NULL, check);
}

static Controller*
make_entry(Lv2ControlID* control)
{
	GtkWidget* entry = gtk_entry_new();

	gtk_widget_set_sensitive(entry, control->is_writable);
	if (control->is_writable) {
		g_signal_connect(G_OBJECT(entry), "activate",
		                 G_CALLBACK(string_changed), control);
	}

	return new_controller(NULL, entry);
}

static Controller*
make_file_chooser(Lv2ControlID* record)
{
	GtkWidget* button = gtk_file_chooser_button_new(
		"Open File", GTK_FILE_CHOOSER_ACTION_OPEN);

	gtk_widget_set_sensitive(button, record->is_writable);

	if (record->is_writable) {
		g_signal_connect(G_OBJECT(button), "file-set",
		                 G_CALLBACK(file_changed), record);
	}

	return new_controller(NULL, button);
}

static Controller*
make_controller(Lv2ControlID* control, float value)
{
	Controller* controller = NULL;

	if (control->is_toggle) {
		controller = make_toggle(control, value);
	} else if (control->is_enumeration) {
		controller = make_combo(control, value);
	} else if (control->is_logarithmic) {
		controller = make_log_slider(control, value);
	} else {
		controller = make_slider(control, value);
	}

	return controller;
}

static GtkWidget*
new_label(const char* text, bool title, float xalign, float yalign)
{
	GtkWidget*  label = gtk_label_new(NULL);
	const char* fmt   = title ? "<span font_weight=\"bold\">%s</span>" : "%s:";
	gchar*      str   = g_markup_printf_escaped(fmt, text);
	gtk_label_set_markup(GTK_LABEL(label), str);
	g_free(str);
	gtk_misc_set_alignment(GTK_MISC(label), xalign, yalign);
	return label;
}

static void
add_control_row(GtkWidget*  table,
                int         row,
                const char* name,
                Controller* controller)
{
	GtkWidget* label = new_label(name, false, 1.0, 0.5);
	gtk_table_attach(GTK_TABLE(table),
	                 label,
	                 0, 1, row, row + 1,
	                 GTK_FILL, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), 8, 1);
	int control_left_attach = 1;
	if (controller->spin) {
		control_left_attach = 2;
		gtk_table_attach(GTK_TABLE(table), GTK_WIDGET(controller->spin),
		                 1, 2, row, row + 1,
		                 GTK_FILL, GTK_FILL, 2, 1);
	}
	gtk_table_attach(GTK_TABLE(table), controller->control,
	                 control_left_attach, 3, row, row + 1,
	                 (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), GTK_FILL, 2, 1);
}

static int
control_group_cmp(const void* p1, const void* p2, void* data)
{
	const Lv2ControlID* control1 = *(const Lv2ControlID**)p1;
	const Lv2ControlID* control2 = *(const Lv2ControlID**)p2;

	const int cmp = (control1->group && control2->group)
		? strcmp(lilv_node_as_string(control1->group),
		         lilv_node_as_string(control2->group))
		: ((intptr_t)control1->group - (intptr_t)control2->group);

	return cmp;
}

static GtkWidget*
build_control_widget(Lv2Plugin* plugin, GtkWidget* window)
{
	GtkWidget* port_table = gtk_table_new(plugin->num_ports, 3, false);

	/* Make an array of controls sorted by group */
	GArray* controls = g_array_new(FALSE, TRUE, sizeof(Lv2ControlID*));
	for (unsigned i = 0; i < plugin->controls.n_controls; ++i) {
		g_array_append_vals(controls, &plugin->controls.controls[i], 1);
	}
	g_array_sort_with_data(controls, control_group_cmp, plugin);

	/* Add controls in group order */
	LilvNode* last_group = NULL;
	int       n_rows     = 0;
	for (size_t i = 0; i < controls->len; ++i) {
		Lv2ControlID*  record     = g_array_index(controls, Lv2ControlID*, i);
		Controller* controller = NULL;
		LilvNode*   group      = record->group;

		/* Check group and add new heading if necessary */
		if (group && !lilv_node_equals(group, last_group)) {
			LilvNode* group_name = lilv_world_get(
				LILV_WORLD, group, plugin->nodes.lv2_name, NULL);
			GtkWidget* group_label = new_label(
				lilv_node_as_string(group_name), true, 0.0f, 1.0f);
			gtk_table_attach(GTK_TABLE(port_table), group_label,
			                 0, 2, n_rows, n_rows + 1,
			                 GTK_FILL, GTK_FILL, 0, 6);
			++n_rows;
		}
		last_group = group;

		/* Make control widget */
		if (record->value_type == plugin->forge.String) {
			controller = make_entry(record);
		} else if (record->value_type == plugin->forge.Path) {
			controller = make_file_chooser(record);
		} else {
			const float val = get_float(record->def, 0.0f);
			controller = make_controller(record, val);
		}

		record->widget = controller;
		if (record->type == PORT) {
			plugin->ports[record->index].widget = controller;
		}
		if (controller) {
			/* Add row to table for this controller */
			add_control_row(
				port_table, n_rows++,
				(record->label
				 ? lilv_node_as_string(record->label)
				 : lilv_node_as_uri(record->node)),
				controller);

			/* Set tooltip text from comment, if available */
			LilvNode* comment = lilv_world_get(
				LILV_WORLD, record->node, plugin->nodes.rdfs_comment, NULL);
			if (comment) {
				gtk_widget_set_tooltip_text(controller->control,
				                            lilv_node_as_string(comment));
			}
			lilv_node_free(comment);
		}
	}

	if (n_rows > 0) {
		gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
		GtkWidget* alignment = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
		/*gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 8, 8);*/
                gtk_widget_set_margin_start (
                  GTK_WIDGET (port_table),
                  8);
                gtk_widget_set_margin_end (
                  GTK_WIDGET (port_table),
                  8);
		gtk_container_add(GTK_CONTAINER(alignment), port_table);
		return alignment;
	} else {
		gtk_widget_destroy(port_table);
		GtkWidget* button = gtk_button_new_with_label("Close");
		g_signal_connect_swapped(button, "clicked",
		                         G_CALLBACK(gtk_widget_destroy), window);
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
		return button;
	}
}

static void
build_menu(Lv2Plugin* plugin, GtkWidget* window, GtkWidget* vbox)
{
  GtkWidget* menu_bar  = gtk_menu_bar_new();
  GtkWidget* file      = gtk_menu_item_new_with_mnemonic("_File");
  GtkWidget* file_menu = gtk_menu_new();

  GtkAccelGroup* ag = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(window), ag);

  GtkWidget* save =
    gtk_menu_item_new_with_mnemonic ("_Save");
  GtkWidget* quit =
    gtk_menu_item_new_with_mnemonic ("_Quit");

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), file_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);

  GtkWidget* pset_item   = gtk_menu_item_new_with_mnemonic("_Presets");
  GtkWidget* pset_menu   = gtk_menu_new();
  GtkWidget* save_preset = gtk_menu_item_new_with_mnemonic(
          "_Save Preset...");
  GtkWidget* delete_preset = gtk_menu_item_new_with_mnemonic(
          "_Delete Current Preset...");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(pset_item), pset_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(pset_menu), save_preset);
  gtk_menu_shell_append(GTK_MENU_SHELL(pset_menu), delete_preset);
  gtk_menu_shell_append(GTK_MENU_SHELL(pset_menu),
                        gtk_separator_menu_item_new());
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), pset_item);

  PresetMenu menu = {
          NULL, NULL, GTK_MENU(pset_menu),
          g_sequence_new((GDestroyNotify)pset_menu_free)
  };
  lv2_load_presets(plugin, add_preset_to_menu, &menu);
  finish_menu(&menu);

  g_signal_connect(G_OBJECT(quit), "activate",
                   G_CALLBACK(on_quit_activate), window);

  g_signal_connect(G_OBJECT(save), "activate",
                   G_CALLBACK(on_save_activate), plugin);

  g_signal_connect(G_OBJECT(save_preset), "activate",
                   G_CALLBACK(on_save_preset_activate), plugin);

  g_signal_connect(G_OBJECT(delete_preset), "activate",
                   G_CALLBACK(on_delete_preset_activate), plugin);

  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
}

void
on_external_ui_closed(void* controller)
{
  Lv2Plugin* jalv = (Lv2Plugin *) controller;
  lv2_close_ui(jalv);
}

bool
lv2_discover_ui(Lv2Plugin* plugin)
{
  return TRUE;
}

int
lv2_open_ui(Lv2Plugin* plugin)
{
  LV2_External_UI_Host extui;
  GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  plugin->window = window;
  extui.ui_closed = on_external_ui_closed;
  LilvNode* name = lilv_plugin_get_name(plugin->lilv_plugin);
  extui.plugin_human_id = lv2_strdup(lilv_node_as_string(name));
  lilv_node_free (name);

  g_signal_connect(window, "destroy",
                   G_CALLBACK(on_window_destroy), plugin);

  set_window_title(plugin);

  GtkWidget* vbox = new_box(false, 0);
  gtk_window_set_role(GTK_WINDOW(window), "plugin_ui");
  gtk_container_add(GTK_CONTAINER(window), vbox);

  if (!no_menu) {
          build_menu(plugin, window, vbox);
  }

  /* Create/show alignment to contain UI (whether custom or generic) */
  GtkWidget* alignment = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
  gtk_box_pack_start(GTK_BOX(vbox), alignment, TRUE, TRUE, 0);
  gtk_widget_show(alignment);

  /* Attempt to instantiate custom UI if necessary */
  if (plugin->ui && !generic_ui)
    {
      if (plugin->externalui)
        {
          g_message ("Instantiating external UI...");
          lv2_ui_instantiate(plugin,
                             lilv_node_as_uri(plugin->ui_type),
                             &extui);
        }
      else
        {
          g_message ("Instantiating native UI...");
          lv2_ui_instantiate(plugin,
                             lv2_native_ui_type(plugin),
                             alignment);
        }
    }

  if (plugin->externalui && plugin->extuiptr)
    {
      LV2_EXTERNAL_UI_SHOW(plugin->extuiptr);
    }
  else if (plugin->ui_instance)
    {
      g_message ("Creating window for UI...");
      GtkWidget* widget = (GtkWidget*)suil_instance_get_widget(
              plugin->ui_instance);

      gtk_container_add(GTK_CONTAINER(alignment), widget);
      gtk_window_set_resizable(GTK_WINDOW(window), lv2_ui_is_resizable(plugin));
      gtk_widget_show_all(vbox);
      gtk_widget_grab_focus(widget);
      gtk_window_present(GTK_WINDOW(window));
    }
  else
    {
      g_message ("No UI found, building native..");
      GtkWidget* controls   = build_control_widget(plugin, window);
      GtkWidget* scroll_win = gtk_scrolled_window_new(NULL, NULL);
      gtk_scrolled_window_add_with_viewport(
              GTK_SCROLLED_WINDOW(scroll_win), controls);
      gtk_scrolled_window_set_policy(
              GTK_SCROLLED_WINDOW(scroll_win),
              GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_container_add(GTK_CONTAINER(alignment), scroll_win);
      gtk_widget_show_all(vbox);

      GtkRequisition controls_size, box_size;
      size_request(GTK_WIDGET(controls), &controls_size);
      size_request(GTK_WIDGET(vbox), &box_size);

      gtk_window_set_default_size(
              GTK_WINDOW(window),
              MAX(MAX(box_size.width, controls_size.width) + 24, 640),
              box_size.height + controls_size.height);
      gtk_window_present(GTK_WINDOW(window));
  }

  lv2_init_ui(plugin);

  g_timeout_add(1000 / plugin->ui_update_hz, (GSourceFunc)lv2_update, plugin);

  return 0;
}

int
lv2_close_ui(Lv2Plugin* plugin)
{
  if (plugin->window)
    {
      g_idle_add ((GSourceFunc) gtk_window_close,
                  plugin->window);
    }
  plugin->window = NULL;
  return TRUE;
}

#if GTK_MAJOR_VERSION == 3
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#    pragma GCC diagnostic pop
#endif
#endif
