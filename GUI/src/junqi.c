#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "junqi.h"
#include "board.h"


int OsRead(int fd, void *zBuf, int iAmt, long iOfst)
{
  off_t ofst;
  int nRead;

  ofst = lseek(fd, iOfst, SEEK_SET);
  if( ofst!=iOfst ){
    return 0;
  }
  nRead = read(fd, zBuf, iAmt);

  return nRead;
}

void LoadChess(Junqi *pJunqi, enum ChessColor color)
{
	int iWidth, iHeight;
	GdkPixbuf *pColor;
	int i;

	iWidth = 36;
	iHeight = 27;
	pColor = pJunqi->ChessImage[color];

	pJunqi->Chess[color][NONE] = NULL;
	pJunqi->Chess[color][DARK] = gdk_pixbuf_new_subpixbuf(pColor,0,0,iWidth,iHeight);
	pJunqi->Chess[color][JUNQI] = gdk_pixbuf_new_subpixbuf(pColor,9*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][DILEI] = gdk_pixbuf_new_subpixbuf(pColor,11*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][ZHADAN] = gdk_pixbuf_new_subpixbuf(pColor,12*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][SILING] = gdk_pixbuf_new_subpixbuf(pColor,10*iWidth,0,iWidth,iHeight);
	for(i=JUNZH; i<=GONGB; i++)
	{
		pJunqi->Chess[color][i] = gdk_pixbuf_new_subpixbuf(pColor,(i-5)*iWidth,0,iWidth,iHeight);
	}


}
void LoadChessImage(Junqi *pJunqi)
{
    int i=0;
	pJunqi->ChessImage[ORANGE] = gdk_pixbuf_new_from_file("./res/orange.bmp",NULL);
	pJunqi->ChessImage[PURPLE] = gdk_pixbuf_new_from_file("./res/purple.bmp",NULL);
	pJunqi->ChessImage[GREEN] = gdk_pixbuf_new_from_file("./res/green.bmp",NULL);
	pJunqi->ChessImage[BLUE] = gdk_pixbuf_new_from_file("./res/blue.bmp",NULL);
	for(i=0; i<4; i++)
	{
		LoadChess(pJunqi,i);
	}

}

void InitLineup(Junqi *pJunqi, enum ChessColor color)
{
	int fd;
	u8 aBuf[4096];
	Jql *pLineup;
	int i;
	fd = open("./res/5.jql", O_RDWR|O_CREAT, 0600);
	OsRead(fd, aBuf, 4096, 0);
	pLineup = (Jql*)(&aBuf[0]);
	if( memcmp(pLineup->aMagic, aMagic, 4)!=0 )
	{
		assert(0);
	}
	for(i=0; i<30; i++)
	{
		pJunqi->Lineup[color][i].type = pLineup->chess[i];
	}

}
void SetChessImageType(Junqi *pJunqi, int color, int i, int iType)
{
	GdkPixbuf *pPixbuf;
	GdkPixbuf *pRotate;
	GtkWidget *pImage;
	pPixbuf = pJunqi->Chess[color][iType];
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[color][i].pImage[0] = pImage;
	pJunqi->Lineup[color][i].pImage[2] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
	//g_object_unref (pPixbuf);//这里pPixbuf是某个图片的子图片，释放掉会出错
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[color][i].pImage[1] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_UPSIDEDOWN);
	g_object_unref (pPixbuf);
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[color][i].pImage[3] = pImage;
	g_object_unref (pPixbuf);
}

void SetChess(Junqi *pJunqi, enum ChessColor color)
{
	enum ChessType iType;
	GtkWidget *pImage;

	int x,y;
	int i;

	for(i=0;i<30;i++)
	{
		iType = pJunqi->Lineup[color][i].type;

		assert( iType>=NONE && iType<=GONGB );

		switch(color)
		{
		case ORANGE:
			x = 265+(i%5)*40;
			y = 13+(i/5+11)*39;
			break;
		case PURPLE:
			x = 38+(i/5+11)*39;
			y = 242+(4-i%5)*39;
			break;
		case GREEN:
			x = 265+(4-i%5)*40;
			y = 13+(5-i/5)*39;
			break;
		case BLUE:
			x = 38+(5-i/5)*39;
			y = 242+(i%5)*39;

			break;
		default:
			assert(0);
			break;
		}
		pJunqi->ChessPos[color][i].xPos = x;
		pJunqi->ChessPos[color][i].yPos = y;
		pJunqi->ChessPos[color][i].pLineup = &pJunqi->Lineup[color][i];
		pJunqi->ChessPos[color][i].type = pJunqi->Lineup[color][i].type;
		if(iType!=NONE)
		{
			SetChessImageType(pJunqi, color, i, iType);
			for(int j=1; j<4; j++)
			{
				pImage = pJunqi->Lineup[color][i].pImage[j];
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, x,y);
			}
			pImage = pJunqi->Lineup[color][i].pImage[color];
			gtk_widget_show(pImage);
		}

	}

}
BoardChess *GetChessPos(Junqi *pJunqi, int x, int y)
{
	BoardChess *pChess = NULL;
	int iDir;
	int iPosX;
	int iPosY;
	int iPos;

	//这里数字的最大值不能乱改，因为是向下取整，改大了会触发断言
	if( (x>=MIDX) && (x<= MIDX+HORIZONTAL_LONG1)  )
	{
		if( (y>=BOTTOMY) && (y<=BOTTOMY+VERTICAL_LONG) )
		{
			iDir = HOME;
			iPosX = (x-MIDX)/LENGTH2;
			iPosY = (y-BOTTOMY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=5);
			if( (x<=MIDX+(iPosX+1)*LENGTH2) && (y<=BOTTOMY+(iPosY+1)*LENGTH1) )
			{
				iPos = iPosY*5 + iPosX;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (y>=TOPY) && (y<=TOPY+VERTICAL_LONG) )
		{
			iDir = OPPS;
			iPosX = (x-MIDX)/LENGTH2;
			iPosY = (y-TOPY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=5);
			if( (x<=MIDX+(iPosX+1)*LENGTH2) && (y<=TOPY+(iPosY+1)*LENGTH1) )
			{
				iPos = (5-iPosY)*5 + 4-iPosX;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (y>=NINE_GRID_Y) && (y<=NINE_GRID_Y+HORIZONTAL_LONG2) )
		{
			iDir = HOME;
			iPosX = (x-NINE_GRID_X)/(2*LENGTH2);
			iPosY = (y-NINE_GRID_Y)/(2*LENGTH1);
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=MIDX+(iPosX*2+1)*LENGTH2) && (y<=NINE_GRID_Y+(iPosY*2+1)*LENGTH1) )
			{
				iPos = iPosY*3 + iPosX;
				pChess = &pJunqi->NineGrid[iPos];
			}
		}
	}
	else if( (y>=MIDY) && (y<=MIDY+HORIZONTAL_LONG2)  )
	{
		if((x>=RIGHTX) && (x<= RIGHTX+VERTICAL_LONG))
		{
			iDir = RIGHT;
			iPosX = (x-RIGHTX)/LENGTH1;
			iPosY = (y-MIDY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=5);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=RIGHTX+(iPosX+1)*LENGTH1) && (y<=MIDY+(iPosY+1)*LENGTH1) )
			{
				iPos = iPosX*5 + 4-iPosY;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (x>=LEFTX) && (x<= LEFTX+VERTICAL_LONG) )
		{
			iDir = LEFT;
			iPosX = (x-LEFTX)/LENGTH1;
			iPosY = (y-MIDY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=5);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=LEFTX+(iPosX+1)*LENGTH1) && (y<=MIDY+(iPosY+1)*LENGTH1) )
			{
				iPos = (5-iPosX)*5 + iPosY;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
	}
	if( pChess )
	{
		pChess->iDir = iDir;
	}

	return pChess;
}

void HideChess(BoardChess *pChess, int iDir)
{
	if(pChess->type!=NONE)
	{
		gtk_widget_hide(pChess->pLineup->pImage[iDir]);
	}
}
void ShowRectangle(Junqi *pJunqi, BoardChess *pChess, int color)
{
	GtkWidget *image;
	int x,y;
	//上家和下家，用竖框
	if(pChess->iDir&1)
	{
		x = pChess->xPos-4;
		y = pChess->yPos-10;
	}
	else
	{
		x = pChess->xPos-4;
		y = pChess->yPos-4;
	}
	if( color==RECTANGLE_RED )
	{
		image = pJunqi->redRectangle[pChess->iDir&1];
	}
	else
	{
		image = pJunqi->whileRectangle[pChess->iDir&1];
	}
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
			                 image,
		                     x,y);
	gtk_widget_show(image);
}

void deal_mouse_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

	int x,y;
	BoardChess *pChess;
	BoardChess *pSrc, *pDst;
	Junqi *pJunqi = (Junqi *)data;
	x = event->x;
	y = event->y;

	pChess=GetChessPos(pJunqi,x,y);
	if( pChess==NULL )
	{
		return;
	}

	if(!pJunqi->bSelect)
	{
		if( pChess->type==NONE )
		{
			return;
		}
		pJunqi->bSelect = 1;
		pJunqi->pSelect = pChess;

		//显示白色选择框
		gtk_widget_hide(pJunqi->redRectangle[0]);
		gtk_widget_hide(pJunqi->redRectangle[1]);

		ShowRectangle(pJunqi, pChess, RECTANGLE_WHITE);
	}
	else
	{
		gtk_widget_hide(pJunqi->whileRectangle[0]);
		gtk_widget_hide(pJunqi->whileRectangle[1]);
		if(pChess==pJunqi->pSelect)
		{
			pJunqi->bSelect = 0;
			pJunqi->pSelect = NULL;
			return;
		}
		pJunqi->bSelect = 0;
		pDst = pChess;
		HideChess(pDst,pChess->iDir);
		pSrc = pJunqi->pSelect;
		pDst->pLineup = pSrc->pLineup;

		gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				       pSrc->pLineup->pImage[pDst->iDir],
				       pDst->xPos,pDst->yPos);
		//注意！这部分代码有很强的顺序关系
		pDst->type = pSrc->type;
		//隐藏选中的棋子
		HideChess(pSrc,pSrc->iDir);
		pSrc->type = NONE;

		gtk_widget_show(pSrc->pLineup->pImage[pDst->iDir]);

		ShowRectangle(pJunqi, pChess, RECTANGLE_RED);
	}

}

void InitSelectImage(Junqi *pJunqi)
{
	pJunqi->whileRectangle[0] = GetSelectImage(0, 0);
	pJunqi->whileRectangle[1] = GetSelectImage(1, 0);
	pJunqi->redRectangle[0] = GetSelectImage(0, 1);
	pJunqi->redRectangle[1] = GetSelectImage(1, 1);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whileRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whileRectangle[1], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[1], 0, 0);
}

void InitBoardItem(Junqi *pJunqi)
{
	InitSelectImage(pJunqi);
}

void InitNineGrid(Junqi *pJunqi)
{
	int i;
	for(i=0; i<9; i++)
	{
		pJunqi->NineGrid[i].xPos = NINE_GRID_X+(i%3)*2*LENGTH2;
		pJunqi->NineGrid[i].yPos = NINE_GRID_Y+(i/3)*2*LENGTH1;
		pJunqi->NineGrid[i].type = NONE;
		pJunqi->NineGrid[i].iDir = HOME;

	}

}
void CreatBoardChess(GtkWidget *window, GtkWidget *fixed)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));
	pJunqi->fixed = fixed;
	g_signal_connect(gtk_widget_get_parent(fixed), "button-press-event",
			G_CALLBACK(deal_mouse_press), pJunqi);

	LoadChessImage(pJunqi);
	for(int i=0; i<4; i++)
	{
		InitLineup(pJunqi,i);
		SetChess(pJunqi,i);
	}
	InitNineGrid(pJunqi);
	InitBoardItem(pJunqi);
}
