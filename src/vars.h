GtkWidget *mainwin;//main window
GtkStatusIcon* trayicon;//tray icon
GtkWidget *convert;//convert button
GtkWidget *stop;//stop button
GtkWidget *pause_continue;//pause and continue button
GtkWidget *output_path;//output path's entry
GtkWidget *list;//main list
GtkListStore *store;//list store
GtkWidget *a_bitrate_spin;
GtkWidget *a_bitrate_auto;
GtkWidget *a_codec_combo;//audio sampling rate
GtkWidget *a_sr_combo;//audio sampling rate
GtkWidget *a_channel_combo;//audio channels
GtkWidget *a_volume_spin;
GtkWidget *a_volume_auto;
GtkWidget *threads_combo;//number of threads
GtkWidget *progress_bar;//progress bar
GtkWidget *in_file_info;//file information

enum {
  COLUMN_NAME,
  COLUMN_STATUS,
  COLUMN_PATH,
  N_COLUMNS
};

struct stats {
float time;
unsigned int total_hours;
unsigned int total_minutes;
float total_seconds;
double total_time;
} stats;

struct states {
GPid pid;//process pid of encoder
gboolean paused;//paused or not paused transcoding
gboolean stoped;//stoped or not stoped transcoding
char last_input_folder[100];
char last_output_folder[100];
unsigned short notebook_tab;
char ffmpeg_path[100];
char lib_path[100];
unsigned short windows_is_visible;
} states;

//--------------setting up codecs(start)----------------//
typedef struct acodec{
char* codec_name;
char* combo_name;
char* extension;
}acodec;

acodec  acodecs[C_NUM_A_CODECS];
