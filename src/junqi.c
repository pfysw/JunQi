#include "junqi.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int OsRead(
  int fd,
  void *zBuf,
  int iAmt,
  long iOfst
){
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
	if( color==ORANGE || color==GREEN )
	{
		iWidth = 36;
		iHeight = 27;
	}
	else
	{
		iWidth = 27;
		iHeight = 36;
	}
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
		pJunqi->ChessPos[color][i].type = pLineup->chess[i];
	}

}
void SetChess(Junqi *pJunqi, enum ChessColor color)
{
	enum ChessType iType;
	GtkWidget *pImage;
	GdkPixbuf *pPixbuf;
	int i;

	for(i=0;i<30;i++)
	{
		iType = pJunqi->ChessPos[color][i].type;
		if( iType!=NONE )
		{
			assert( iType>NONE && iType<=GONGB );
			pPixbuf = pJunqi->Chess[color][iType];
			pImage = gtk_image_new_from_pixbuf(pPixbuf);
			pJunqi->ChessPos[color][i].pImage = pImage;
			switch(color)
			{
			case ORANGE:
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, 265+(i%5)*40,13+(i/5+11)*39);
				break;
			case PURPLE:
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, 38+(i/5+11)*39,242+(4-i%5)*39);
				break;
			case GREEN:
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, 265+(4-i%5)*40,13+(5-i/5)*39);
				break;
			case BLUE:
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, 38+(5-i/5)*39,242+(i%5)*39);
				break;
			default:
				break;
			}
		}
	}

}

void CreatBoardChess(GtkWidget *fixed)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	pJunqi->fixed = fixed;
	LoadChessImage(pJunqi);
	for(int i=0; i<4; i++)
	{
		InitLineup(pJunqi,i);
		SetChess(pJunqi,i);
	}
}
