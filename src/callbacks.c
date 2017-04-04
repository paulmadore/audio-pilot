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

void on_tray_icon_activate(GtkStatusIcon* icon, gpointer user_data)
{
	if(states.windows_is_visible)
	{
		states.windows_is_visible = 0;
		gtk_widget_hide(mainwin);
	}
	else
	{
		states.windows_is_visible = 1;
		gtk_widget_show(mainwin);
	}
}
gboolean on_exit_program(GtkWidget *widget, gpointer user_data)
{
	if(states.stoped)
	{
		save_config();
		gtk_main_quit();
		return FALSE;
	}
	else
	{
		popup("An operation is still in progress.");
		return TRUE;
	}
}
void on_convert_btn_clicked(GtkWidget *widget, gpointer user_data)
{
	GTimer* timer=g_timer_new();
	
	GtkTreeSelection* selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
	
	GtkTreeModel* model=gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(model,&iter);
	
	char * envp[]={states.lib_path,NULL};
	
	if(valid)
	{
		states.stoped=0;
		enable_disable_widgets();
	}
	
	while(valid && !states.stoped)
	{
		//scroll an selection stuff
		GtkTreePath* t_path=gtk_tree_model_get_path(GTK_TREE_MODEL(model),&iter);
		if(t_path)
		{
			gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(list), t_path, NULL, FALSE, 0.0, 0.0 );
			gtk_tree_path_free(t_path);
		}
		gtk_tree_selection_select_iter(selection,&iter);
		
		char* input_file;//should be freed
		char* output_file;//should be freed
		
		gtk_tree_model_get(model, &iter,COLUMN_PATH, &input_file,-1);//getting input file
		
		//making full path for output file
		char* out_folder = get_out_path();
		char* base_name = get_base_name(input_file);
		
		output_file = calloc( strlen(out_folder) + 1 + strlen(base_name) + 5 + 1,sizeof(char) );
		strcpy( output_file , out_folder );
		strcat( output_file , "/" );
		strcat( output_file , base_name );
		strcat( output_file , acodecs[ gtk_combo_box_get_active(GTK_COMBO_BOX(a_codec_combo)) ].extension );
		
		//-------------------------------------------preparing array(start)------------------------------------------------//
			//should be freed
			char* a_sr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(a_sr_combo));
			char* a_channel = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(a_channel_combo));
			char* threads_number = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(threads_combo));
			
			unsigned int audio_bitrate = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(a_bitrate_spin) ) * 1000;
			char a_bitrate[ count_int(audio_bitrate) + 1 ];
			sprintf(a_bitrate,"%d",audio_bitrate);
			
			unsigned int audio_volume = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(a_volume_spin) );
			char a_volume[ count_int(audio_volume) + 1 ];
			sprintf(a_volume,"%d",audio_volume);
			
			unsigned short arr_size = 13;//we must be careful here
			
			//amr encoding requires bitrate,sample rate and number of channels
			unsigned short is_amr = 0;
			if( strcmp( "libopencore_amrnb",acodecs[gtk_combo_box_get_active(GTK_COMBO_BOX(a_codec_combo))].codec_name)==0 )
			{
				is_amr = 1;
			}
			
			unsigned short is_amr_wb = 0;
			if( strcmp( "libvo_amrwbenc",acodecs[gtk_combo_box_get_active(GTK_COMBO_BOX(a_codec_combo))].codec_name)==0 )
			{
				is_amr_wb = 1;
			}

			// sample rate for amr is required even for 'auto' mode
			if(gtk_combo_box_get_active(GTK_COMBO_BOX(a_sr_combo))!=0 || is_amr || is_amr_wb)
			{
				arr_size+=2;
			}
			
			// number of channels for amr is required even for 'auto' mode
			if(gtk_combo_box_get_active(GTK_COMBO_BOX(a_channel_combo))!=0 || is_amr || is_amr_wb)
			{
				arr_size+=2;
			}
			
			//manual audio volume
			if( !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a_volume_auto)) )
			{
				arr_size+=2;
			}
			
		//-------------------------------------------preparing array(end)------------------------------------------------//
		
		//------------------------------------------filling array (start)--------------------------------------------------//
			unsigned short arr_index=0;
			char * args[arr_size];
			
			//ffmpeg path
			args[arr_index] = states.ffmpeg_path;
				arr_index++;
				
			//input file
			args[arr_index]="-i";
				arr_index++;
			args[arr_index]=input_file;
				arr_index++;
				
			//no video
			args[arr_index]="-vn";
				arr_index++;
				
			//number of threads
			args[arr_index]="-threads";
				arr_index++;
			args[arr_index]=threads_number;
				arr_index++;
				
			//audio codec
			args[arr_index]="-acodec";
				arr_index++;
			args[arr_index]=acodecs[gtk_combo_box_get_active(GTK_COMBO_BOX(a_codec_combo))].codec_name;
				arr_index++;
			
			//sampling rate
			if(gtk_combo_box_get_active(GTK_COMBO_BOX(a_sr_combo))!=0 || is_amr || is_amr_wb)
			{
				args[arr_index]="-ar";
					arr_index++;
				if(is_amr)
				{
					args[arr_index]="8000";
				}
				else if(is_amr_wb)
				{
					args[arr_index]="16000";
				}
				else
				{
					args[arr_index]=a_sr;
				}
				arr_index++;
			}
			
			//audio bitrate
			args[arr_index]="-ab";
				arr_index++;
			if(is_amr)
			{
				args[arr_index] = "12200";
			}
			else if( !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a_bitrate_auto)) )
			{
				args[arr_index]=a_bitrate;
			}
			else
			{
				args[arr_index] = C_DEFAULT_AUDIO_BITRATE_STR;
			}
			arr_index++;
			
			//audio channels
			if(gtk_combo_box_get_active(GTK_COMBO_BOX(a_channel_combo))!=0 || is_amr || is_amr_wb)
			{
				args[arr_index]="-ac";
					arr_index++;
				if(is_amr || is_amr_wb)
				{
					args[arr_index]="1";
				}
				else
				{
					args[arr_index]=a_channel;
				}
				arr_index++;
			}
			
			//manual audio volume
			if( !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a_volume_auto)) )
			{
				args[arr_index]="-vol";
					arr_index++;
				args[arr_index]=a_volume;
					arr_index++;
			}
			
			//overwrite if file exists
			args[arr_index]="-y";
				arr_index++;
			
			args[arr_index] = output_file;//output file name
				arr_index++;
			
			args[arr_index]=NULL;
		//------------------------------------------filling array (end)--------------------------------------------------//
		
		print_str_arr(args);//printing command for debugging
		
		gboolean done = FALSE;//if the job is done
		
		gint fd_err;
		gint fd_out;
		gint fd_in;
		
		GError *err = NULL;
		
		g_spawn_async_with_pipes(NULL,args,envp,G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH,NULL,NULL,&states.pid,&fd_in,&fd_out,&fd_err,&err);
		
		//arguments are passed to child, so time for free()
		free(input_file);
		free(output_file);
		free(out_folder);
		free(base_name);
		
		free(a_sr);
		free(a_channel);
		free(threads_number);
		//-----------------------------//
		
		if(err!=NULL)
		{
			fprintf(stderr,"%s\n",err->message);
			g_error_free(err);
			exit(1);
		}
		
		if(states.paused)
		{
			setpriority(PRIO_PROCESS,states.pid,PRIO_MAX);
		}
		
		GIOChannel *io_err=g_io_channel_unix_new(fd_err);
		g_io_channel_set_encoding (io_err,NULL,NULL);
		
		GString *errstr = g_string_new("");
		
		gboolean show_stat=FALSE;//we are not ready for stats
		
		g_timer_start(timer);
		
		unsigned short iter_number = 0;
		
		while( waitpid(states.pid,NULL,WNOHANG)==0 && !states.stoped)
		{		
			if(done)
			{
				usleep(1000);
			}
			else
			{
				usleep(33);
			}
			
			if(!show_stat)
			{
				g_io_channel_read_line_string(io_err,errstr,NULL,NULL);
				g_print(errstr->str);
				
				if(sscanf(errstr->str," Duration: %u:%u:%g",&stats.total_hours,&stats.total_minutes,&stats.total_seconds)==3)
				{
					show_stat=TRUE;
					stats.total_time=stats.total_hours*3600+stats.total_minutes*60+stats.total_seconds;
				}
			}
			else
			{
				if(done)
				{
					iter_number++;
					if(iter_number==500)
					{
						iter_number = 0;//reset
						
						g_io_channel_read_line_string(io_err,errstr,NULL,NULL);
						g_print(errstr->str);
						
						done = proceed_stat(errstr->str,&iter,timer);
					}
				}
				else
				{
					g_io_channel_read_line_string(io_err,errstr,NULL,NULL);
					g_print(errstr->str);
					
					done = proceed_stat(errstr->str,&iter,timer);
				}
			}
			
			while(gtk_events_pending()) 
			{
				gtk_main_iteration();
			}
		}
		
		if(states.stoped && waitpid(states.pid,NULL,WNOHANG)!=0)
		{
			if( kill(states.pid,SIGINT)!=0)
			{
				kill(states.pid,SIGKILL);
			}
			states.pid=0;
		}
		
		g_io_channel_shutdown(io_err,TRUE,&err);
		
		g_string_free(errstr, TRUE);
		
		if(done)
		{
			gtk_list_store_set(store,&iter,COLUMN_STATUS,"Done",-1);
		}
		else
		{
			gtk_list_store_set(store,&iter,COLUMN_STATUS,"Fail",-1);
		}
		
		valid = gtk_tree_model_iter_next(model, &iter);
	}
	
	g_timer_destroy(timer);
	
	states.pid=0;
	states.stoped=1;
	enable_disable_widgets();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar),"");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar),(gdouble)0);
	gtk_status_icon_set_tooltip_text(trayicon, "Transcoder Audio Edition");
	
	printf("\nEnd\n");
}
void on_stop_btn_clicked(GtkWidget *widget, gpointer user_data)
{
	states.stoped=1;
}
void on_pause_continue_btn_clicked(GtkWidget *widget, gpointer user_data)
{
	if(states.pid)
	{
		if(!states.paused)
		{
			gtk_button_set_label(GTK_BUTTON(pause_continue),"Faster");
			gtk_widget_set_tooltip_text(GTK_WIDGET(pause_continue),"Increase resource usage");
			states.paused=1;
			setpriority(PRIO_PROCESS,states.pid,PRIO_MAX);
		}
		else
		{
			gtk_button_set_label(GTK_BUTTON(pause_continue),"Slower");
			gtk_widget_set_tooltip_text(GTK_WIDGET(pause_continue),"Decrease resource usage");
			states.paused=0;
			setpriority(PRIO_PROCESS,states.pid,PRIO_USER);
		}
	}
}
void on_list_drag_data_received(GtkWidget *widget,GdkDragContext *drag_ctx,gint x,gint y,GtkSelectionData  *data,guint32 info,guint32 time)
{
	g_signal_stop_emission_by_name(widget, "drag-data-received");
	
	char** uris = gtk_selection_data_get_uris(data);
	if(uris!=NULL)
	{
		char** uri;
		for(uri = uris; *uri; ++uri)
		{
			char* fn = g_filename_from_uri(*uri, NULL, NULL);
			add_to_list(list,fn);
			free(fn);
		}
		g_strfreev(uris);
	}
}
void on_item_del(GtkWidget *widget, gpointer user_data)
{
	if(states.stoped)
	{
		GtkTreeIter iter;
		GtkTreeModel *model;
		GtkTreeSelection* selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
		if(gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model, &iter))
		{
			if(gtk_tree_model_iter_next(model,&iter))
			{
				gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model, &iter);
				gtk_list_store_remove(GTK_LIST_STORE(store),&iter);
				gtk_tree_selection_select_iter(selection,&iter);
			}
			else
			{
				gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model, &iter);
				gtk_list_store_remove(GTK_LIST_STORE(store),&iter);
			}
		}
	}
}
void on_list_clear(GtkWidget *widget, gpointer user_data)
{
	if(states.stoped)
	{
		gtk_list_store_clear(GTK_LIST_STORE(store));
	}
}
void on_about(GtkWidget* widget, gpointer user_data)
{
	const gchar* authors[] = {"Arman Poghosyan (chelovek84) <chelovek84@yahoo.com>",NULL};
	const gchar* artists[] = {"Levon Ohanyan (Leo) <ohanyanlevon@gmail.com>",NULL};
	GtkWidget* about = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about),"Transcoder Audio Edition");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about),C_VERSION);
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about),authors);
	gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about),artists);
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about)," This program is free software; you can redistribute it and/or \n modify it under the terms of the GNU General Public License \n as published by the Free Software Foundation; either version 3 \n of the License, or any later version. \n This program is distributed in the hope that it will be useful, \n but WITHOUT ANY WARRANTY; without even the implied warranty of \n MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \n GNU General Public License for more details. \n You should have received a copy of the GNU General Public License \n along with this program; if not, write to the Free Software \n Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.");
	gtk_window_set_transient_for(GTK_WINDOW(about),GTK_WINDOW(mainwin));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(about),TRUE);
	gtk_dialog_run(GTK_DIALOG(about));
	gtk_widget_destroy(about);
}

void on_item_select(GtkWidget* widget, gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection* selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
	char* in_file;
	if(gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection),&model, &iter))
	{
		
		gtk_tree_model_get(model, &iter,COLUMN_PATH, &in_file,-1);
		
		char * arg_x[4];
		arg_x[0] = states.ffmpeg_path;
		arg_x[1] = "-i";
		arg_x[2] = in_file;
		arg_x[3] = NULL;
		
		char * envp[]={states.lib_path,NULL};
		
		char* output;
		if( g_spawn_sync(NULL,arg_x,envp,G_SPAWN_STDOUT_TO_DEV_NULL,NULL,NULL,NULL,&output,NULL,NULL) )
		{
			char* first_p = strstr(output,"Duration");
			char* last_p = strstr(output,"At least one output file must be specified");
			
			if( first_p!=NULL && last_p!=NULL && last_p-first_p > 0 )
			{
				char info_str[ last_p - first_p + 1 ];
				
				strncpy(info_str,first_p,last_p-first_p);
				
				info_str[last_p - first_p] = '\0';
				
				GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(in_file_info));
				
				static gboolean tag_lable_exists;
				
				if(!tag_lable_exists)
				{
					gtk_text_buffer_create_tag(buffer,"small_font","size-points",8.5,NULL);
					tag_lable_exists = TRUE;
				}
				
				gtk_text_buffer_set_text(buffer,"",-1);
				
				GtkTextIter iter;
				gtk_text_buffer_get_iter_at_line(buffer, &iter, 0);
				
				gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, info_str, -1, "small_font",NULL);
			}
		}
		
		free(in_file);
		free(output);
	}
}

void on_a_bitrate_auto(GtkWidget* checkbox, gpointer user_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)))
	{
		gtk_widget_set_sensitive(a_bitrate_spin,FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(a_bitrate_spin,TRUE);
	}
}

void on_a_volume_auto(GtkWidget* checkbox, gpointer user_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox)))
	{
		gtk_widget_set_sensitive(a_volume_spin,FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(a_volume_spin,TRUE);
	}
}
