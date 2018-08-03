/*
 * board.c
 *
 *  Created on: Jul 28, 2018
 *      Author: Administrator
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include "junqi.h"

static GdkPixbuf *background;
static Junqi *gJunqi;

typedef struct BoardItem
{
	GtkWidget *button[4];
}BoardItem;

BoardItem board;

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

/*
 * 设置菜单栏，暂时不用，待扩展
 */
void set_menu(GtkWidget *vbox)
{
	GtkWidget *menubar,*menu,*menuitem;

    menubar=gtk_menu_bar_new();
    gtk_widget_set_hexpand (menubar, TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);
   // gtk_fixed_put(GTK_FIXED(vbox), menubar, 0, 0);
	menuitem=gtk_menu_item_new_with_label("文件");
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), menuitem);

	menu=gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	menuitem=gtk_menu_item_new_with_label("新建...");
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect(GTK_MENU_ITEM(menuitem),"activate",G_CALLBACK(event_handle),NULL);
}

/*
 * 把背景图片显示在画布上，如果要要把画布显示到界面，此回调函数是必须的
 */
static gint draw_cb (
	 GtkWidget *widget,
	 cairo_t   *cr,
	 gpointer   data)
{
  gdk_cairo_set_source_pixbuf (cr, background, 0, 0);
  cairo_paint (cr);

  return TRUE;
}

/*
 * 根据坐标和长度截取pixbuf，并返回图片控件
 */
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

static void button_cb(GtkWidget *button , gpointer data)
{
    char *zDir = (char*)data;
    Junqi *pJunqi = gJunqi;
    GtkFileChooserNative *native;

    if( strcmp(zDir,"home" )==0 )
    {
    	pJunqi->selectDir = HOME;
    }
    else if( strcmp(zDir,"right" )==0 )
    {
    	pJunqi->selectDir = RIGHT;
    }
    else if( strcmp(zDir,"opps" )==0 )
    {
    	pJunqi->selectDir = OPPS;
    }
    else if( strcmp(zDir,"left" )==0 )
    {
    	pJunqi->selectDir = LEFT;
    }

	native = gtk_file_chooser_native_new ("Open File",
										NULL,
										GTK_FILE_CHOOSER_ACTION_OPEN,
										"_Open",
										"_Cancel");
	gtk_native_dialog_show (GTK_NATIVE_DIALOG (native));
	pJunqi->native = native;
	g_signal_connect (native,
					"response",
					G_CALLBACK (select_chess_cb),
					pJunqi);
}

/*
 * 设置调入布局按钮，每家都有一个
 */
void SetButton(GtkWidget *window, Junqi *pJunqi)
{
	GtkWidget **button = board.button;
	GError*  error =NULL;
	GdkPixbuf *pixbuf;
	GtkWidget *fixed = pJunqi->fixed;

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
	    	g_signal_connect (button[i], "clicked", G_CALLBACK (button_cb), "home");
	    	break;
	    case 1:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 614,202);
	    	g_signal_connect (button[i], "clicked", G_CALLBACK (button_cb), "right");
	    	break;
	    case 2:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 169,40);
	    	g_signal_connect (button[i], "clicked", G_CALLBACK (button_cb), "opps");
	    	break;
	    case 3:
	    	gtk_fixed_put(GTK_FIXED(fixed), button[i], 60,452);
	    	g_signal_connect (button[i], "clicked", G_CALLBACK (button_cb), "left");
	    	break;
	    default:
	    	break;
	    }

	}
}

/*
 * 画一个矩形方框，用来选中棋子
 * isVertical：表示横竖
 * color：表示是白色还是红色
 */
GtkWidget *GetSelectImage(int isVertical, int color)
{
	cairo_t *cr;
	cairo_surface_t *surface = NULL;
	GdkPixbuf* pixbuf;
	GtkWidget *image;
	//新建一块画布，大小随意
	surface = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, 800, 900) ;
	cr = cairo_create (surface);

    cairo_set_line_width (cr, 4);
    if(color)
    {
        //红色方框
    	cairo_set_source_rgb (cr, 255, 0, 0);
    }
    else
    {
    	cairo_set_source_rgb (cr, 180, 238, 180);
    }
    cairo_rectangle (cr, 4, 4, 36, 27);

    cairo_stroke(cr);
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);

    pixbuf = gdk_pixbuf_get_from_surface(surface,0,0,50,50);
    if(isVertical)
    {
    	GdkPixbuf* src = pixbuf;
    	pixbuf = gdk_pixbuf_rotate_simple(src,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
    	g_object_unref (src);
    }
	image = gtk_image_new_from_pixbuf(pixbuf);

    cairo_surface_destroy (surface);
    cairo_destroy (cr);
    g_object_unref (pixbuf);

    return image;
}




static void begin_button(GtkWidget *button, GdkEventButton *event, gpointer data)
{
	Junqi *pJunqi = (Junqi *)data;
	//隐藏调入布局按钮
	for(int i=0; i<4; i++)
	{
		gtk_widget_hide(board.button[i]);
	}

	if(!pJunqi->bStart)
	{
		pJunqi->bStart = 1;
        //将开始按钮替换为新按钮，点击后将会立即出招 todo
		GdkPixbuf* pixbuf;
		GtkWidget *image1;
		GtkWidget *fixed =gtk_widget_get_parent(button);
		gtk_container_remove(GTK_CONTAINER(button),pJunqi->data);
		pixbuf = gdk_pixbuf_new_from_file_at_scale(
				"./res/start2.png", 40,40,1,NULL);
		image1 = gtk_image_new_from_pixbuf(pixbuf);
		gtk_container_add(GTK_CONTAINER(button),image1);
		gtk_fixed_move( GTK_FIXED(fixed), button, 560,560);
		gtk_widget_show(image1);
	}

}

void OpenBoard(GtkWidget *window)
{
	GtkWidget* draw_area = gtk_drawing_area_new();
	GtkWidget *vbox, *hbox, *vbox2;
	Junqi *pJunqi = JunqiOpen();
	gJunqi = pJunqi;
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (window), hbox);
    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox2, FALSE, TRUE, 0);
    set_menu(vbox);

    GtkWidget *event_box = gtk_event_box_new();
    gtk_box_pack_start (GTK_BOX (vbox), event_box, TRUE, TRUE, 0);
    //fixed用来存放各种容器，把fixed放在事件盒子里，来处理鼠标事件
    GtkWidget *fixed = gtk_fixed_new();
    pJunqi->fixed = fixed;
    gtk_container_add(GTK_CONTAINER(event_box),fixed);
    gtk_widget_set_size_request(draw_area, 733,688);
    gtk_fixed_put(GTK_FIXED(fixed), draw_area, 0, 0);
    background = gdk_pixbuf_new_from_file("./res/1024board.bmp", NULL);
    g_signal_connect (draw_area, "draw",G_CALLBACK (draw_cb), NULL);


    /*棋盘右边区域，扩展用*/
//    draw_area = gtk_drawing_area_new();
//    gtk_widget_set_size_request(draw_area, 133,688);
    //gtk_box_pack_start (GTK_BOX (hbox), draw_area, TRUE, TRUE, 0);

    //开始按钮
    GtkWidget *button_box = gtk_event_box_new();//gtk_button_new();
    //gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(
    		"./res/begin.gif", 100,100,1,NULL);
    GtkWidget *image = gtk_image_new_from_pixbuf(pixbuf);
    gtk_container_add(GTK_CONTAINER(button_box),image);
   // gtk_button_set_image(GTK_BUTTON(button), image);

    pJunqi->data = image;
    g_signal_connect(button_box,"button-press-event",G_CALLBACK(begin_button),pJunqi);
    gtk_fixed_put(GTK_FIXED(fixed), button_box, 560,560);
    SetButton(window,pJunqi);

    gtk_widget_show_all(window);

    CreatBoardChess(window, pJunqi);
}
