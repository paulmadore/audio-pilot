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

void setup_settings(void)
{
	short counter = 0;
	
	acodecs[counter].codec_name="libvorbis";
	acodecs[counter].combo_name="Vorbis";
	acodecs[counter].extension=".ogg";
	counter++;
	
	acodecs[counter].codec_name="libvo_aacenc";
	acodecs[counter].combo_name="Aac";
	acodecs[counter].extension=".mp4";
	counter++;
	
	acodecs[counter].codec_name="libmp3lame";
	acodecs[counter].combo_name="Mp3";
	acodecs[counter].extension=".mp3";
	counter++;
	
	acodecs[counter].codec_name="mp2";
	acodecs[counter].combo_name="Mp2";
	acodecs[counter].extension=".mp2";
	counter++;
	
	acodecs[counter].codec_name="libopencore_amrnb";
	acodecs[counter].combo_name="AmrNB";
	acodecs[counter].extension=".amr";
	counter++;
	
	acodecs[counter].codec_name="libvo_amrwbenc";
	acodecs[counter].combo_name="AmrWB";
	acodecs[counter].extension=".amr";
	counter++;
	
	acodecs[counter].codec_name="flac";
	acodecs[counter].combo_name="Flac";
	acodecs[counter].extension=".flac";
	counter++;
	
}
void setup_states(void)
{
	states.pid=0;
	states.paused=0;
	states.stoped=1;
	states.notebook_tab=0;
	
	if( g_file_test("ffmpeg",G_FILE_TEST_IS_EXECUTABLE) )
	{
		strcpy( states.ffmpeg_path , "./ffmpeg" );
	}
	else if(g_file_test("/usr/lib/TranscoderAudioEdition/ffmpeg",G_FILE_TEST_IS_EXECUTABLE))
	{
		strcpy( states.ffmpeg_path , "/usr/lib/TranscoderAudioEdition/ffmpeg" );
	}
	else if(g_file_test("/usr/bin/ffmpeg",G_FILE_TEST_IS_EXECUTABLE))
	{
		strcpy( states.ffmpeg_path , "/usr/bin/ffmpeg" );
	}
	else if(g_file_test("/usr/local/bin/ffmpeg",G_FILE_TEST_IS_EXECUTABLE))
	{
		strcpy( states.ffmpeg_path , "/usr/local/bin/ffmpeg" );
	}
	else
	{
		printf("\n could not find ffmpeg \n\n");
		exit(2);
	}
	printf("ffmpeg: %s \n" , states.ffmpeg_path);
	
	if( g_file_test("libs",G_FILE_TEST_IS_DIR) )
	{
		strcpy(states.lib_path,"LD_LIBRARY_PATH=libs");
	}
	else if( g_file_test("/usr/lib/TranscoderAudioEdition",G_FILE_TEST_IS_DIR) )
	{
		strcpy(states.lib_path,"LD_LIBRARY_PATH=/usr/lib/TranscoderAudioEdition");
	}
	else
	{
		strcpy(states.lib_path,"");
	}
	printf("libraries: %s \n" , states.lib_path);
	
	states.windows_is_visible = 1;
}
//--------------setting up codecs(end)----------------//
int main (int argc, char **argv)
{
	setup_settings();
	
	setup_states();
	
	gtk_init (&argc, &argv);
	
	init_ui();
	
	load_config();
	
	gtk_widget_show_all(mainwin);
	
	gtk_main();
	
	return 0;
}
