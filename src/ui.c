#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#define GTK_DISABLE_DEPRECATED 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "misc.h"
#include "callbacks.h"
#include "ui.h"

#include "vars.h"

void init_ui(void)
{
	char icon_path[100];
	if( g_file_test("data/transcoderaudioedition.png",G_FILE_TEST_IS_REGULAR) )
	{
		strcpy( icon_path , "data/transcoderaudioedition.png" );
	}
	else if( g_file_test("/usr/share/TranscoderAudioEdition/transcoderaudioedition.png",G_FILE_TEST_IS_REGULAR) )
	{
		strcpy( icon_path , "/usr/share/TranscoderAudioEdition/transcoderaudioedition.png" );
	}
	else if( g_file_test("/usr/local/share/TranscoderAudioEdition/transcoderaudioedition.png",G_FILE_TEST_IS_REGULAR) )
	{
		strcpy( icon_path , "/usr/local/share/TranscoderAudioEdition/transcoderaudioedition.png" );
	}
	else
	{
		strcpy( icon_path , "" );
	}
	//------------------------window--------------------------//
			mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title(GTK_WINDOW(mainwin),C_WINDOW_TITLE);
			gtk_window_set_default_size(GTK_WINDOW(mainwin),C_WINDOW_WIDTH,C_WINDOW_HEIGHT);
			gtk_container_set_border_width(GTK_CONTAINER(mainwin),1);
			gtk_window_set_position(GTK_WINDOW(mainwin),GTK_WIN_POS_CENTER);
			gtk_window_set_icon_from_file(GTK_WINDOW(mainwin),icon_path,NULL);
			g_signal_connect(GTK_WIDGET(mainwin),"delete-event",G_CALLBACK(on_exit_program),NULL);
	//------------------------window--------------------------//
	
	
	
	//------------------------system tray--------------------------//
			trayicon = gtk_status_icon_new_from_file(icon_path);
			gtk_status_icon_set_tooltip_text(trayicon, "Transcoder Audio Edition");
			g_signal_connect(trayicon, "activate", G_CALLBACK(on_tray_icon_activate), NULL);
	//------------------------system tray--------------------------//
	
	
	
	//------------------------menubar--------------------------//
			GtkAccelGroup* accel_group = gtk_accel_group_new();
			gtk_window_add_accel_group(GTK_WINDOW(mainwin),accel_group);
			
			GtkWidget* menu_bar=gtk_menu_bar_new();
			
			//----------------file menu(start)----------------//
			GtkWidget* file_menu=gtk_menu_new();
			
			GtkWidget* menu_file=gtk_menu_item_new_with_label("File");
				gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar),menu_file);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_file),file_menu);
			
			GtkWidget* menu_open=gtk_menu_item_new_with_label("Open");
				gtk_menu_shell_append(GTK_MENU_SHELL(file_menu),menu_open);
				g_signal_connect(G_OBJECT(menu_open),"activate",G_CALLBACK(add_files),NULL);
			
			GtkWidget* menu_exit=gtk_menu_item_new_with_label("Exit");
				gtk_menu_shell_append(GTK_MENU_SHELL(file_menu),menu_exit);
				g_signal_connect(G_OBJECT(menu_exit),"activate",G_CALLBACK(on_exit_program),NULL);
				gtk_widget_add_accelerator(menu_exit,"activate",accel_group,GDK_KEY_q,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
			//----------------file menu(end)----------------//
			
			//----------------edit menu(start)----------------//
			GtkWidget* edit_menu=gtk_menu_new();
			
			GtkWidget* menu_edit=gtk_menu_item_new_with_label("Edit");
				gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar),menu_edit);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_edit),edit_menu);
				
			GtkWidget* menu_remove=gtk_menu_item_new_with_label("Remove selected");
				gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu),menu_remove);
				g_signal_connect(G_OBJECT(menu_remove),"activate",G_CALLBACK(on_item_del),NULL);
				gtk_widget_add_accelerator(menu_remove,"activate",accel_group,GDK_KEY_Delete,GDK_LOCK_MASK,GTK_ACCEL_VISIBLE);
			GtkWidget* menu_clear_all=gtk_menu_item_new_with_label("Clear all");
				gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu),menu_clear_all);
				g_signal_connect(G_OBJECT(menu_clear_all),"activate",G_CALLBACK(on_list_clear),NULL);
				gtk_widget_add_accelerator(menu_clear_all,"activate",accel_group,GDK_KEY_Delete,GDK_SHIFT_MASK,GTK_ACCEL_VISIBLE);
			//----------------edit menu(end)----------------//
			
			//----------------help menu(start)----------------//
			GtkWidget* help_menu=gtk_menu_new();
			
			GtkWidget* menu_help=gtk_menu_item_new_with_label("Help");
				gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar),menu_help);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_help),help_menu);
				
			GtkWidget* menu_about=gtk_menu_item_new_with_label("About");
				gtk_menu_shell_append(GTK_MENU_SHELL(help_menu),menu_about);
				g_signal_connect(G_OBJECT(menu_about),"activate",G_CALLBACK(on_about),NULL);
			//----------------help menu(end)----------------//
	//------------------------menubar--------------------------//
	
	
	
	//------------------------toolbar--------------------------//
			GtkWidget* add=gtk_button_new_from_stock(GTK_STOCK_ADD);
			g_signal_connect(G_OBJECT(add),"clicked",G_CALLBACK(add_files),NULL);
			
			convert=gtk_button_new_from_stock(GTK_STOCK_CONVERT);
			g_signal_connect(G_OBJECT(convert),"clicked",G_CALLBACK(on_convert_btn_clicked),NULL);
			
			stop=gtk_button_new_from_stock(GTK_STOCK_STOP);
			g_signal_connect(G_OBJECT(stop),"clicked",G_CALLBACK(on_stop_btn_clicked),NULL);
			gtk_widget_set_sensitive(stop,FALSE);
			
			GtkWidget* separator=gtk_separator_new(GTK_ORIENTATION_VERTICAL);
			
			pause_continue=gtk_button_new_with_label("Slower");
			g_signal_connect(G_OBJECT(pause_continue),"clicked",G_CALLBACK(on_pause_continue_btn_clicked),NULL);
			gtk_widget_set_sensitive(pause_continue,FALSE);
			
			GtkWidget *empty_space=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			
			GtkWidget *output_path_lable=gtk_label_new("Output folder");
			
			output_path=gtk_entry_new();
			gtk_editable_set_editable(GTK_EDITABLE(output_path),FALSE);
			
			GtkWidget* browse = gtk_button_new_with_label("Browse");
			g_signal_connect(G_OBJECT(browse),"clicked",G_CALLBACK(browse_path),NULL);
			
			GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(add), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(convert), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(stop), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(separator), FALSE, FALSE, 3);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(pause_continue), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(empty_space), TRUE, TRUE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(output_path_lable), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(output_path), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(toolbar), GTK_WIDGET(browse), FALSE, FALSE, 1);
	//------------------------toolbar--------------------------//
	
	
	
	//------------------------list--------------------------//
			GtkWidget* sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			
			list = gtk_tree_view_new();//list in scrolled window
			gtk_container_add(GTK_CONTAINER(sw),list);
			gtk_drag_dest_set(GTK_WIDGET(list),GTK_DEST_DEFAULT_ALL,NULL,100,GDK_DRAG_ENTER);
			gtk_drag_dest_add_uri_targets(GTK_WIDGET(list));
			g_signal_connect(list, "drag_data_received", G_CALLBACK(on_list_drag_data_received), NULL);
			g_signal_connect (list,"row-activated",G_CALLBACK(on_item_select), NULL);
			
			GtkCellRenderer *renderer;
			renderer = gtk_cell_renderer_text_new();
			
			GtkTreeViewColumn *column_name = gtk_tree_view_column_new_with_attributes("Name",renderer, "text", COLUMN_NAME, NULL);
			gtk_tree_view_column_set_min_width(column_name,150);
			gtk_tree_view_column_set_resizable(column_name,TRUE);
			gtk_tree_view_append_column(GTK_TREE_VIEW(list),column_name);
			
			GtkTreeViewColumn *column_status = gtk_tree_view_column_new_with_attributes("Status",renderer, "text", COLUMN_STATUS, NULL);
			gtk_tree_view_column_set_min_width(column_status,50);
			gtk_tree_view_column_set_resizable(column_status,TRUE);
			gtk_tree_view_append_column(GTK_TREE_VIEW(list),column_status);
			
			GtkTreeViewColumn *column_path = gtk_tree_view_column_new_with_attributes("Path",renderer, "text", COLUMN_PATH, NULL);
			gtk_tree_view_append_column(GTK_TREE_VIEW(list),column_path);
			
			store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
			gtk_tree_view_set_model(GTK_TREE_VIEW(list),GTK_TREE_MODEL(store));
			g_object_unref(store);
	//------------------------list--------------------------//
	
	
	progress_bar=gtk_progress_bar_new();
	
	
	//-------------------------audio settings(start)-----------------------//
			gshort for_iter;
			
			GtkWidget *a_codec_lable=gtk_label_new("Codec");
			gtk_widget_set_size_request(a_codec_lable,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(a_codec_lable),0,0.5);
			
			a_codec_combo=gtk_combo_box_text_new();
			for(for_iter=0;for_iter<=C_NUM_A_CODECS-1;for_iter++)
			{
				gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_codec_combo),acodecs[for_iter].combo_name);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_codec_combo),0);
				
				GtkWidget *a_codec_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(a_codec_box),a_codec_lable,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_codec_box),a_codec_combo,FALSE,FALSE,1);
			
			GtkWidget *a_bitrate_lable=gtk_label_new("Bitrate");
			gtk_widget_set_size_request(a_bitrate_lable,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(a_bitrate_lable),0,0.5);
			
			a_bitrate_auto = gtk_check_button_new_with_label("Default");
			g_signal_connect (a_bitrate_auto,"toggled",G_CALLBACK(on_a_bitrate_auto), NULL);
			
			a_bitrate_spin=gtk_spin_button_new_with_range(C_MIN_AUDIO_BITRATE,C_MAX_AUDIO_BITRATE,16);
			gtk_editable_set_editable(GTK_EDITABLE(a_bitrate_spin),FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(a_bitrate_spin),C_DEFAULT_AUDIO_BITRATE);
			
				GtkWidget *a_bitrate_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(a_bitrate_box),a_bitrate_lable,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_bitrate_box),a_bitrate_spin,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_bitrate_box),a_bitrate_auto,FALSE,FALSE,1);
			
			GtkWidget* a_sr_lable=gtk_label_new("Sampling rate");
			gtk_widget_set_size_request(a_sr_lable,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(a_sr_lable),0,0.5);
			
			a_sr_combo=gtk_combo_box_text_new();
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"Auto");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"8000");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"11025");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"16000");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"22050");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"44100");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_sr_combo),"48000");
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_sr_combo),6);
			
				GtkWidget *a_sr_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(a_sr_box),a_sr_lable,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_sr_box),a_sr_combo,FALSE,FALSE,1);
			
			GtkWidget* a_channel_lable=gtk_label_new("Channels");
			gtk_widget_set_size_request(a_channel_lable,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(a_channel_lable),0,0.5);
			
			a_channel_combo=gtk_combo_box_text_new();
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_channel_combo),"Auto");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_channel_combo),"1");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_channel_combo),"2");
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(a_channel_combo),"6");
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_channel_combo),2);
			
				GtkWidget *a_channel_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(a_channel_box),a_channel_lable,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_channel_box),a_channel_combo,FALSE,FALSE,1);
				
			GtkWidget *a_volume_label=gtk_label_new("Volume");
			gtk_widget_set_size_request(a_volume_label,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(a_volume_label),0,0.5);
			
			a_volume_auto = gtk_check_button_new_with_label("Auto");
			g_signal_connect (a_volume_auto,"toggled",G_CALLBACK(on_a_volume_auto), NULL);
			
			a_volume_spin=gtk_spin_button_new_with_range(C_MIN_AUDIO_VOLUME,C_MAX_AUDIO_VOLUME,16);
			gtk_editable_set_editable(GTK_EDITABLE(a_volume_spin),FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(a_volume_spin),C_DEFAULT_AUDIO_VOLUME);
			
				GtkWidget *a_volume_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(a_volume_box),a_volume_label,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_volume_box),a_volume_spin,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(a_volume_box),a_volume_auto,FALSE,FALSE,1);

				
			GtkWidget *threads_lable=gtk_label_new("Threads");
			gtk_widget_set_size_request(threads_lable,C_LABLE_WIDTH,C_LABLE_HEIGHT);
			gtk_misc_set_alignment(GTK_MISC(threads_lable),0,0.5);
			
			threads_combo=gtk_combo_box_text_new();
			char threads_num_char[2];
			for(for_iter=1;for_iter<=8;for_iter++)
			{
				sprintf(threads_num_char,"%d",for_iter);
				gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(threads_combo),threads_num_char);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(threads_combo),0);
				
				GtkWidget *threads_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,1);
				gtk_box_pack_start(GTK_BOX(threads_box),threads_lable,FALSE,FALSE,1);
				gtk_box_pack_start(GTK_BOX(threads_box),threads_combo,FALSE,FALSE,1);
	//-------------------------audio settings(end)-----------------------//
	
	//----------------------audio settings boxes(start)------------//
	GtkWidget *audio_box=gtk_box_new(GTK_ORIENTATION_VERTICAL,3);
	gtk_container_set_border_width(GTK_CONTAINER(audio_box),5);
	gtk_box_pack_start(GTK_BOX(audio_box),a_codec_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(audio_box),a_bitrate_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(audio_box),a_sr_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(audio_box),a_channel_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(audio_box),a_volume_box,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(audio_box),threads_box,FALSE,FALSE,0);
	
	//----------------------audio settings boxes(end)------------//
	
	//------------------------text area--------------------------//
			GtkWidget* in_file_info_sw = gtk_scrolled_window_new(NULL, NULL);
			gtk_container_set_border_width(GTK_CONTAINER(in_file_info_sw),2);
			gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(in_file_info_sw),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			
			in_file_info = gtk_text_view_new();//in_file_info in scrolled window
			gtk_text_view_set_editable(GTK_TEXT_VIEW(in_file_info),FALSE);
			gtk_container_add(GTK_CONTAINER(in_file_info_sw),in_file_info);
			
			gtk_widget_set_size_request(GTK_WIDGET(in_file_info_sw), C_WINDOW_HEIGHT, 100);
	//------------------------text area--------------------------//
	
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(toolbar), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(sw), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(progress_bar), FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(audio_box), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(in_file_info_sw), FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(mainwin), vbox);
}
