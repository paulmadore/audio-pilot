char* get_base_name(char* path);
char* get_full_name(char* path);
char* get_out_path(void);
void print_str_arr(gchar** str_arr);//prints array of strings
void popup(gchar *mes);//creates popup window with message
void enable_disable_widgets();//disables or enables some widgets
void add_files(void);//creates file chooser dialog
void browse_path(void);//creates folder chooser dialog
void add_to_list(GtkWidget *list, gchar *str);//adds files to list
gboolean proceed_stat(gchar *str,GtkTreeIter* iter,GTimer* timer);//show statistics
void save_config(void);//save configuration
void load_config(void);//load configuration
unsigned short count_int(unsigned int number);
