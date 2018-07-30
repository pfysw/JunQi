/*
 * board.c
 *
 *  Created on: Jul 28, 2018
 *      Author: Administrator
 */
#include <gtk/gtk.h>
#include <stdlib.h>

static GdkPixbuf *background;

void CreatBoardChess(GtkWidget *fixed);

typedef struct Board
{
	GtkWidget *button[4];
}Board;

Board board;

static void event_handle(GtkWidget *item,gpointer data)
{
	char *event = (char*)data;
	if( event==NULL )
	{
		return;
	}
	if( strcmp(event,"new" )==0 )
	{
		printf("new!\n");
	}

}

void set_menu(GtkWidget *vbox)
{
	GtkWidget *menubar,*menu,*menuitem;

    menubar=gtk_menu_bar_new();
    gtk_widget_set_hexpand (menubar, TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);

	menuitem=gtk_menu_item_new_with_label("文件");
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menu=gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	menuitem=gtk_menu_item_new_with_label("新建...");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(GTK_MENU_ITEM(menuitem),"activate",G_CALLBACK(event_handle),NULL);
}

static gint draw_cb (
	 GtkWidget *widget,
	 cairo_t   *cr,
	 gpointer   data)
{
  gdk_cairo_set_source_pixbuf (cr, background, 0, 0);
  cairo_paint (cr);

  return TRUE;
}


GtkWidget *get_image_from_pixbuf(GdkPixbuf* pixbuf,
		gint src_x,
		gint src_y,
		gint width,
		gint height )
{
	GtkWidget *image;
	GdkPixbuf* pTemp;
	pTemp=gdk_pixbuf_new_subpixbuf(pixbuf, src_x,src_y,width,height);
	image = gtk_image_new_from_pixbuf(pTemp);
	g_object_unref (pTemp);
	return image;
}

void SetButton(GtkWidget *window, GtkWidget *fixed)
{
	GtkWidget **button = board.button;
	GError*  error =NULL;
	GdkPixbuf *pixbuf;

	char *name = "./res/load.bmp";
	int i;
	for(i=0; i<4; i++)
	{
		button[i] = gtk_button_new();

		pixbuf = gdk_pixbuf_new_from_file(name, &error);
		if(!pixbuf) {
			fprintf(stderr, "board image open error!!!!\n%s\n",name);
			exit(1);
		}
		GtkWidget *image = get_image_from_pixbuf(pixbuf, 0,0,77,22);

		gtk_button_set_image(GTK_BUTTON(button[i]), image);
		gtk_button_set_relief(GTK_BUTTON(button[i]),GTK_RELIEF_NONE);
	    switch(i)
	    {
	    case 0:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 470,621);
	    	break;
	    case 1:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 614,202);
	    	break;
	    case 2:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 169,40);
	    	break;
	    case 3:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 60,452);
	    	break;
	    default:
	    	break;
	    }

	}
}

static void begin_button(GtkWidget *button, gpointer data)
{
	for(int i=0; i<4; i++)
	{
		gtk_widget_hide(board.button[i]);
	}
	gtk_button_set_label(GTK_BUTTON(button),"走棋");
}

void BoardInit(GtkWidget *window)
{
	GtkWidget* draw_area = gtk_drawing_area_new();
	GtkWidget *vbox, *hbox, *vbox2;

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (window), hbox);
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);
    //set_menu(vbox2);

    GtkWidget *fixed = gtk_fixed_new();
    gtk_box_pack_start (GTK_BOX (vbox), fixed, TRUE, TRUE, 0);
    gtk_widget_set_size_request(draw_area, 733,688);
    gtk_fixed_put(GTK_FIXED(fixed), draw_area, 0, 0);
    background = gdk_pixbuf_new_from_file("./res/1024board.bmp", NULL);
    g_signal_connect (draw_area, "draw",G_CALLBACK (draw_cb), NULL);

    /*棋盘右边区域，扩展用*/
//    draw_area = gtk_drawing_area_new();
//    gtk_widget_set_size_request(draw_area, 133,688);
    //gtk_box_pack_start (GTK_BOX (hbox), draw_area, TRUE, TRUE, 0);


    GtkWidget *button = gtk_button_new_with_label("开始");
    g_signal_connect(GTK_BUTTON(button),"clicked",G_CALLBACK(begin_button),fixed);
    gtk_fixed_put(GTK_FIXED(fixed), button, 50,50);
    SetButton(window,fixed);

    CreatBoardChess(fixed);

}
