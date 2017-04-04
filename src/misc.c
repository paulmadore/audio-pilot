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

char* get_base_name(char* path)
{
	char* basename;
	char* base_name_iter;
	char* path_iter;
	char* last_slash = strrchr(path,'/');
	char* last_dot = strrchr(path,'.');
	
	if(last_slash==NULL)
	{
		if(last_dot==NULL)
		{
			basename=calloc( strlen(path)+1 , sizeof(char) );
			strcpy(basename,path);
		}
		else
		{
			basename=calloc( last_dot-path+1 , sizeof(char) );
			base_name_iter=basename;
			path_iter=path;
			while(path_iter!=last_dot)
			{
				*base_name_iter=*path_iter;
				base_name_iter++;
				path_iter++;
			}
			*base_name_iter='\0';
		}
	}
	else
	{
		if( last_dot==NULL || (last_dot-last_slash) < 0 )
		{
			basename = calloc( strlen(path)-(last_slash-path)+1,sizeof(char) );
			base_name_iter = basename;

			path_iter=last_slash+1;
			while(*path_iter!='\0')
			{
				*base_name_iter=*path_iter;
				base_name_iter++;
				path_iter++;
			}
			*base_name_iter='\0';
		}
		else
		{
			basename = calloc(last_dot-last_slash,sizeof(char));
			base_name_iter = basename;

			path_iter=last_slash+1;
			while(path_iter!=last_dot)
			{
				*base_name_iter=*path_iter;
				base_name_iter++;
				path_iter++;
			}
			*base_name_iter='\0';
		}
	}
	return basename;
}

char* get_full_name(char* path)
{
	char* fullname;
	char* full_name_iter;
	char* path_iter;
	char* last_slash = strrchr(path,'/');
	
	fullname = calloc( strlen(path)-(last_slash-path)+1,sizeof(char) );
	full_name_iter = fullname;

	path_iter=last_slash+1;
	while(*path_iter!='\0')
	{
		*full_name_iter=*path_iter;
		full_name_iter++;
		path_iter++;
	}
	*full_name_iter='\0';
	return fullname;
}

char* get_out_path(void)
{
	char* path;
	const char* selected_path = gtk_entry_get_text(GTK_ENTRY(output_path));
	if(selected_path==NULL || !g_file_test(selected_path,G_FILE_TEST_IS_DIR) || access(selected_path,7)!=0  )
	{
		path = calloc(strlen(getenv("HOME")) + 1 + 6 + 1,sizeof(char));
		strcpy(path,getenv("HOME"));
		strcat(path,"/");
		strcat(path,"Music");
		gtk_entry_set_text(GTK_ENTRY(output_path),path);
		
		if( !g_file_test(path,G_FILE_TEST_IS_DIR) )
		{
			mkdir(path,0755);
		}
		else
		{
			if(access(path,0755)!=0)
			{
				chmod(path,0755);
			}
		}
		
	}
	else
	{
		path=calloc( strlen(selected_path)+1,sizeof(char) );
		strcpy(path,selected_path);
	}
	return path;
}
void add_files()
{
	GtkWidget *input_file_chooser = gtk_file_chooser_dialog_new ("Open File",GTK_WINDOW(mainwin),GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(input_file_chooser),TRUE);
	
	if(states.last_input_folder!=NULL && g_file_test(states.last_input_folder,G_FILE_TEST_IS_DIR))
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(input_file_chooser),states.last_input_folder);
	}
	else
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(input_file_chooser),getenv("HOME"));
	}
	
	if(gtk_dialog_run (GTK_DIALOG (input_file_chooser)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *current_filename;
		GSList *first_filename = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(input_file_chooser));
		for(current_filename = first_filename; current_filename; current_filename = current_filename->next)
		{
			add_to_list(list,current_filename->data);
		}
		g_slist_free(current_filename);
		g_slist_free(first_filename);
	}
	
	char* new_input_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(input_file_chooser));
	if(new_input_folder!=NULL)
	{
		if(strlen(new_input_folder) < 100)
		{
			strcpy(states.last_input_folder,new_input_folder);
		}
		free(new_input_folder);
	}
	
	gtk_widget_destroy(input_file_chooser);
}
void browse_path()
{
	GtkWidget *ouput_file_chooser = gtk_file_chooser_dialog_new ("Open File",GTK_WINDOW(mainwin),GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,NULL);
	
	if(states.last_output_folder!=NULL && g_file_test(states.last_output_folder,G_FILE_TEST_IS_DIR))
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(ouput_file_chooser),states.last_output_folder);
	}
	else
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(ouput_file_chooser),getenv("HOME"));
	}
	
	if(gtk_dialog_run (GTK_DIALOG (ouput_file_chooser)) == GTK_RESPONSE_ACCEPT)
	{
		char *foldername = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ouput_file_chooser));
		if(access(foldername,7)==0)
		{
			gtk_entry_set_text(GTK_ENTRY(output_path),foldername);
		}
		
		free(foldername);
	}
	
	char* new_output_folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(ouput_file_chooser));
	if(new_output_folder!=NULL)
	{
		if(strlen(new_output_folder) < 100)
		{
			strcpy(states.last_output_folder,new_output_folder);
		}
		free(new_output_folder);
	}
	
	gtk_widget_destroy(ouput_file_chooser);
}
void add_to_list(GtkWidget *list, gchar *str)
{
	if(!g_file_test(str,G_FILE_TEST_IS_SYMLINK) && !g_file_test(str,G_FILE_TEST_IS_DIR) && g_file_test(str,G_FILE_TEST_IS_REGULAR))
	{
		GtkTreeIter iter;
		store = GTK_LIST_STORE(  gtk_tree_view_get_model( GTK_TREE_VIEW(list) )  );
		gtk_list_store_append(store, &iter);
		
		char* fullname=get_full_name(str);
		
		gtk_list_store_set(store, &iter, COLUMN_NAME, fullname, COLUMN_STATUS, "Waiting", COLUMN_PATH, str, -1);
		
		free(fullname);
	}
}
gboolean proceed_stat(gchar* str,GtkTreeIter* iter,GTimer* timer)
{
	unsigned int h;
	unsigned int m;
	float s;
	if(   sscanf(str, "size= %*d kB time= %u:%u:%g bitrate= %*g kbits/s",&h, &m, &s) == 3   )
	{
		stats.time = h * 3600 + m * 60 +s;
		
		double elapsed=g_timer_elapsed(timer,NULL);
		unsigned short e_hours=floor(elapsed/3600);
		unsigned short e_minutes=floor(elapsed/60)-60*e_hours;
		unsigned short e_seconds=floor(elapsed-e_hours*3600-e_minutes*60);
		
		char number_src[5];
				
		if(stats.total_time!=0)
		{
			double remain=stats.total_time * elapsed / stats.time - elapsed;
			unsigned short r_hours=floor(remain/3600);
			unsigned short r_minutes=floor(remain/60)-60*r_hours;
			unsigned short r_seconds=floor(remain-r_hours*3600-r_minutes*60);
			
			char stat_string[60];
					
			strcpy(stat_string,"Time elapsed: ");
			
			if(e_hours < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_hours);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(e_minutes < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_minutes);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(e_seconds < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_seconds);
			strcat(stat_string,number_src);
			
			strcat(stat_string,"  \t  Time remained: ");
			
			if(r_hours < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",r_hours);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(r_minutes < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",r_minutes);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(r_seconds < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",r_seconds);
			strcat(stat_string,number_src);
			
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar),stat_string);
			
			gdouble percent=100 * stats.time / stats.total_time;
			
			char progress[5];
			sprintf(progress,"%d",(int)percent);
			strcat(progress,"%");
			gtk_list_store_set(store,iter,COLUMN_STATUS,progress,-1);
			gtk_status_icon_set_tooltip_text(trayicon,progress);
			
			percent=percent / 100;
			if(percent >= 0 && percent <= 1)
			{
				gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar),percent);
			}
		}
		else
		{
			char stat_string[22];
			strcpy(stat_string,"Time elapsed: ");
			
			if(e_hours < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_hours);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(e_minutes < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_minutes);
			strcat(stat_string,number_src);
			strcat(stat_string,":");
			
			if(e_seconds < 10)
			{
				strcat(stat_string,"0");
			}
			sprintf(number_src,"%d",e_seconds);
			strcat(stat_string,number_src);
			
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar),stat_string);
			gtk_list_store_set(store,iter,COLUMN_STATUS,"In process",-1);
		}
		return TRUE;
	}
	else
	{
		if( strstr(str,"muxing overhead")!=NULL )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}
void popup(gchar *mes)
{
	GtkWidget *popwin = gtk_message_dialog_new(GTK_WINDOW(mainwin),GTK_DIALOG_MODAL,GTK_MESSAGE_OTHER,GTK_BUTTONS_OK,mes);
	gtk_dialog_run(GTK_DIALOG(popwin));
	gtk_widget_destroy(popwin);
}
void enable_disable_widgets(void)
{
	if(!states.stoped)
	{
		gtk_widget_set_sensitive(convert,FALSE);
		
		gtk_widget_set_sensitive(stop,TRUE);
		gtk_widget_set_sensitive(pause_continue,TRUE);
	}
	else
	{
		gtk_widget_set_sensitive(convert,TRUE);
		
		gtk_widget_set_sensitive(stop,FALSE);
		gtk_widget_set_sensitive(pause_continue,FALSE);
	}
}
void save_config(void)
{
	gchar* dir = g_build_filename(g_get_user_config_dir(),"Transcoder_Audio_Edition",NULL);
	gchar* path = g_build_filename(dir,"config",NULL);
	g_mkdir_with_parents(dir,0755);
	free(dir);
	FILE* f=fopen(path,"w");
	free(path);
	if(f)
	{
		fprintf(f, "[Config]\n");
		
		fprintf(f,"version=%s\n",C_VERSION);
		
		//--------------------------------audio config(start)--------------------------------//
		fprintf(f,"a_codec=%d\n",gtk_combo_box_get_active(GTK_COMBO_BOX(a_codec_combo)));
		fprintf(f,"a_sr=%d\n",gtk_combo_box_get_active(GTK_COMBO_BOX(a_sr_combo)));
		fprintf(f,"a_channels=%d\n",gtk_combo_box_get_active(GTK_COMBO_BOX(a_channel_combo)));
		fprintf(f,"a_bitrate=%d\n",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(a_bitrate_spin)));
		fprintf( f,"a_bitrate_auto=%d\n",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a_bitrate_auto)) );
		fprintf(f,"a_volume=%d\n",gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(a_volume_spin)));
		fprintf( f,"a_volume_auto=%d\n",gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(a_volume_auto)) );
		//--------------------------------audio config(end)--------------------------------//
		//----------------misc config(start)----------------//
		fprintf(f,"output_path=%s\n",gtk_entry_get_text(GTK_ENTRY(output_path)));
		fprintf(f,"last_input_folder=%s\n",states.last_input_folder);
		fprintf(f,"last_output_folder=%s\n",states.last_output_folder);
		fprintf(f,"threads=%d\n",gtk_combo_box_get_active(GTK_COMBO_BOX(threads_combo)));
		//----------------misc config(end)----------------//
		fclose(f);
	}
}
void load_config(void)
{
	gchar* path = g_build_filename(g_get_user_config_dir(), "Transcoder_Audio_Edition", "config", NULL );
	GKeyFile* kf = g_key_file_new();
	if(	g_key_file_load_from_file(kf,path,0,NULL) )
	{
		gint v;
		const gchar grp[] = "Config";
		
		char* version = g_key_file_get_string(kf, grp, "version", NULL);
		if( version!=NULL && strcmp(version , C_VERSION)==0 )
		{
			free(version);
			
			//--------------------------------audio config(start)--------------------------------//
			v = g_key_file_get_integer(kf, grp, "a_bitrate", NULL);
			if(v >= C_MIN_AUDIO_BITRATE && v <= C_MAX_AUDIO_BITRATE)
			{
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(a_bitrate_spin),v);
				
			}
			
			v = g_key_file_get_integer(kf, grp, "a_bitrate_auto", NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(a_bitrate_auto),v);
			
			v = g_key_file_get_integer(kf, grp, "a_volume", NULL);
			if(v >= C_MIN_AUDIO_VOLUME && v <= C_MAX_AUDIO_VOLUME)
			{
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(a_volume_spin),v);
				
			}
			
			v = g_key_file_get_integer(kf, grp, "a_volume_auto", NULL);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(a_volume_auto),v);
			
			v = g_key_file_get_integer(kf, grp, "a_codec", NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_codec_combo),v);
			
			v = g_key_file_get_integer(kf, grp, "a_sr", NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_sr_combo),v);
			
			v = g_key_file_get_integer(kf, grp, "a_channels", NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(a_channel_combo),v);
			//--------------------------------audio config(end)--------------------------------//
			
			//----------------misc config(start)----------------//
			char* ouput_path = g_key_file_get_string(kf, grp, "output_path", NULL);
			if(ouput_path!=NULL)
			{
				gtk_entry_set_text(GTK_ENTRY(output_path),ouput_path);
				free(ouput_path);
			}
			
			char* last_input_folder = g_key_file_get_string(kf, grp, "last_input_folder", NULL);
			if(last_input_folder!=NULL)
			{
				if(strlen(last_input_folder) < 100)
				{
					strcpy(states.last_input_folder,last_input_folder);
				}
				free(last_input_folder);
			}
			
			char* last_output_folder = g_key_file_get_string(kf, grp, "last_output_folder", NULL);
			if(last_output_folder!=NULL)
			{
				if(strlen(last_output_folder) < 100)
				{
					strcpy(states.last_output_folder,last_output_folder);
				}
				free(last_output_folder);
			}
			
			v = g_key_file_get_integer(kf, grp, "threads", NULL);
			gtk_combo_box_set_active(GTK_COMBO_BOX(threads_combo),v);
			//----------------misc config(end)----------------//
		}
	}
	g_free(path);
	g_key_file_free(kf);
}
unsigned short count_int(unsigned int number)
{
	double work_number=(double)number;
	unsigned short counter = 0;
	while(work_number >= 1)
	{
		work_number=work_number/10;
		counter++;
	}
	return counter;
}
void print_str_arr(gchar** str_arr)
{
	unsigned short i;
	for(i=0; str_arr[i]!=NULL; i++)
	{
		g_print(str_arr[i]);
		g_print(" ");
	}
}
