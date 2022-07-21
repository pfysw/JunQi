#include <gtk/gtk.h>
#include <gdk/gdk.h>

char *version = "2.3";

void OpenBoard(GtkWidget *window);

int main(int argc, char *argv[])
{
    GtkWidget *window;
    //setvbuf(stdout, NULL, _IONBF, 0);
    gtk_init(&argc,&argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "四国军棋");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	//gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy", gtk_main_quit, NULL);
    //初始化界面
	OpenBoard(window);
    gtk_main();
    return 0;
}


