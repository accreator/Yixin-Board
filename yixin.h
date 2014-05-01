#include <cairo.h>
#include <gtk/gtk.h>
void send_command(char *command);
void make_move(int y, int x);
gboolean on_button_press_windowmain(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge);
int is_integer(const char *str);
void show_dialog_settings(GtkWidget *widget, gpointer data);
void show_dialog_load(GtkWidget *widget, gpointer data);
void show_dialog_save(GtkWidget *widget, gpointer data);
void show_dialog_about(GtkWidget *widget, gpointer data);
void new_game(GtkWidget *widget, gpointer data);
void change_rule(GtkWidget *widget, gpointer data);
void change_side(GtkWidget *widget, gpointer data);
void change_side_menu(int flag, GtkWidget *w);
void change_piece(GtkWidget *widget, gpointer data);
GdkPixbuf * copy_subpixbuf(GdkPixbuf *_src, int src_x, int src_y, int width, int height);
void create_windowmain();
gboolean iochannelout_watch(GIOChannel *channel, GIOCondition cond, gpointer data);
gboolean iochannelerr_watch(GIOChannel *channel, GIOCondition cond, gpointer data);
void load_setting();
void load_engine();
void init_engine();
#define max(x, y) ((x)>(y)?(x):(y))
#define min(x, y) ((x)<(y)?(x):(y))
#define VERSION "1.0"