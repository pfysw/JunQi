#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "junqi.h"
#include "board.h"
#include "rule.h"
#include "pthread.h"

void select_flag_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

static const u8 aLineupBuf[] = {
	0x51, 0x51, 0x47, 0x61, 0x6D, 0x65, 0x20, 0x4A,
	0x51, 0x4C, 0x20, 0x46, 0x69, 0x6C, 0x65, 0x00,
	0x57, 0x04, 0x00, 0x00, 0x05, 0x08, 0x0B, 0x0D,
	0x07, 0x09, 0x00, 0x0B, 0x00, 0x09, 0x06, 0x0D,
	0x00, 0x0B, 0x07, 0x04, 0x00, 0x0C, 0x00, 0x0A,
	0x08, 0x03, 0x0C, 0x03, 0x04, 0x0A, 0x02, 0x03,
	0x0C, 0x0D
};

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

int OsWrite(
  int fd,
  void *zBuf,
  int iAmt,
  long iOfst
){
  off_t ofst;
  int nWrite;

  ofst = lseek(fd, iOfst, SEEK_SET);
  if( ofst!=iOfst ){
    return 0;
  }
  nWrite = write(fd, zBuf, iAmt);

  return nWrite;
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
	Jql *pLineup;
	int i;

	pLineup = (Jql*)(&aLineupBuf[0]);

	if( memcmp(pLineup->aMagic, aMagic, 4)!=0 )
	{
		assert(0);
	}
	for(i=0; i<30; i++)
	{
		pJunqi->Lineup[color][i].type = pLineup->chess[i];
	}

}

void SetChessImageType(Junqi *pJunqi, int dir, int i, int iType)
{
	GdkPixbuf *pPixbuf;
	GdkPixbuf *pRotate;
	GtkWidget *pImage;

	pPixbuf = pJunqi->Chess[(dir+pJunqi->eColor)%4][iType];
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	g_object_ref_sink (pImage);
	pJunqi->Lineup[dir][i].pImage[0] = pImage;
	pJunqi->Lineup[dir][i].pImage[2] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
	//g_object_unref (pPixbuf);//这里pPixbuf是某个图片的子图片，释放掉会出错
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	g_object_ref_sink (pImage);
	pJunqi->Lineup[dir][i].pImage[1] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_UPSIDEDOWN);
	g_object_unref (pPixbuf);
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	g_object_ref_sink (pImage);
	pJunqi->Lineup[dir][i].pImage[3] = pImage;
	g_object_unref (pPixbuf);
}

void SetBoardCamp(Junqi *pJunqi, enum ChessDir dir, int i)
{
	if(i==6||i==8||i==12||i==16||i==18)
	{
		pJunqi->ChessPos[dir][i].isCamp = 1;
	}
	else if(i==26||i==28)
	{
		pJunqi->ChessPos[dir][i].isStronghold = 1;
	}
}

void SetBoardRailway(Junqi *pJunqi, enum ChessDir dir, int i)
{
	if(i<25)
	{
		if( (i/5==0||i/5==4) || ((i%5==0||i%5==4)) )
		{
			pJunqi->ChessPos[dir][i].isRailway = 1;
		}
	}
}

void CreatGraphVertex(Junqi *pJunqi, BoardChess *pChess)
{
	AdjNode *pNode;
	BoardGraph *pVertex;
	pNode = (AdjNode *)malloc(sizeof(AdjNode));
	pNode->pChess = pChess;
	pNode->pNext = NULL;
	pVertex = &pJunqi->aBoard[pChess->point.x][pChess->point.y];

	assert( pVertex->pAdjList==NULL );
	pVertex->pAdjList = pNode;
}

void SetChess(Junqi *pJunqi, enum ChessDir dir)
{
	enum ChessType iType;
	GtkWidget *pImage;

	int x,y;
	int i;

	pJunqi->eColor = PURPLE;
	pJunqi->eTurn = (4-pJunqi->eColor)%4;

    //从方阵的左上角从上往下，从左往右遍历
	for(i=0;i<30;i++)
	{
		iType = pJunqi->Lineup[dir][i].type;

		assert( iType>=NONE && iType<=GONGB );
        // 下家最右侧横坐标为0
		// 对家最上面纵坐标为0
		switch(dir)
		{
		case HOME:
			x = 265+(i%5)*40;
			y = 13+(i/5+11)*39;
			pJunqi->ChessPos[dir][i].point.x = 10-i%5;
			pJunqi->ChessPos[dir][i].point.y = 11+i/5;
			break;
		case RIGHT:
			x = 38+(i/5+11)*39;
			y = 242+(4-i%5)*39;
			pJunqi->ChessPos[dir][i].point.x = 5-i/5;
			pJunqi->ChessPos[dir][i].point.y = 10-i%5;
			break;
		case OPPS:
			x = 265+(4-i%5)*40;
			y = 13+(5-i/5)*39;
			pJunqi->ChessPos[dir][i].point.x = 6+i%5;
			pJunqi->ChessPos[dir][i].point.y = 5-i/5;
			break;
		case LEFT:
			x = 38+(5-i/5)*39;
			y = 242+(i%5)*39;
			pJunqi->ChessPos[dir][i].point.x = 11+i/5;
			pJunqi->ChessPos[dir][i].point.y = 6+i%5;
			break;
		default:
			assert(0);
			break;
		}

		pJunqi->ChessPos[dir][i].xPos = x;
		pJunqi->ChessPos[dir][i].yPos = y;
		pJunqi->ChessPos[dir][i].pLineup = &pJunqi->Lineup[dir][i];
		pJunqi->ChessPos[dir][i].type = pJunqi->Lineup[dir][i].type;
		pJunqi->ChessPos[dir][i].index = i;
		pJunqi->ChessPos[dir][i].iDir= dir;
		pJunqi->Lineup[dir][i].pChess = &pJunqi->ChessPos[dir][i];
		pJunqi->Lineup[dir][i].bDead = 0;
		pJunqi->Lineup[dir][i].iDir = dir;
		SetBoardCamp(pJunqi, dir, i);
		SetBoardRailway(pJunqi, dir, i);

		if(iType!=NONE)
		{
#if NOT_DEBUG1
			if( (dir==RIGHT||dir==LEFT) && !pJunqi->bReplay )
			{
				iType = DARK;
			}
#endif
			SetChessImageType(pJunqi, dir, i, iType);
			for(int j=1; j<4; j++)
			{
				pImage = pJunqi->Lineup[dir][i].pImage[j];
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, x,y);
			}
			pImage = pJunqi->Lineup[dir][i].pImage[dir];
			gtk_widget_show(pImage);


			if( pJunqi->Lineup[dir][i].pFlag )
			{
				gtk_widget_destroy(pJunqi->Lineup[dir][i].pFlag);
				GdkPixbuf *pixbuf = pJunqi->pSelect->pLineup->pixbuf;
				pJunqi->Lineup[dir][i].pFlag = gtk_image_new_from_pixbuf(pixbuf);
				MoveFlag(pJunqi,&pJunqi->ChessPos[dir][i],0);
			}

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
			if( (x<=MIDX+iPosX*LENGTH2+36) && (y<=BOTTOMY+iPosY*LENGTH1+27) )
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
			if( (x<=MIDX+iPosX*LENGTH2+36) && (y<=TOPY+iPosY*LENGTH1+27) )
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
			if( (x<=MIDX+iPosX*2*LENGTH2+36) && (y<=NINE_GRID_Y+iPosY*2*LENGTH1+27) )
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
			if( (x<=RIGHTX+iPosX*LENGTH1+27) && (y<=MIDY+iPosY*LENGTH1+36) )
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
			if( (x<=LEFTX+iPosX*LENGTH1+27) && (y<=MIDY+iPosY*LENGTH1+36) )
			{
				iPos = (5-iPosX)*5 + iPosY;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
	}

	return pChess;
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
		image = pJunqi->whiteRectangle[pChess->iDir&1];
	}
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
			                 image,
		                     x,y);
	gtk_widget_show(image);
}

void MoveFlag(Junqi *pJunqi, BoardChess *pChess, int isInit)
{
	int flagX,flagY;
	if(pChess->iDir&1)
	{
		flagX = pChess->xPos+3;
		flagY = pChess->yPos+7;
	}
	else
	{
		flagX = pChess->xPos+7;
		flagY = pChess->yPos+2;
	}

	if(isInit)
	{
		gtk_fixed_move(GTK_FIXED(pJunqi->fixed),pChess->pLineup->pFlag,flagX,flagY);
	}
	else
	{
		gtk_fixed_put(GTK_FIXED(pJunqi->fixed),pChess->pLineup->pFlag,flagX,flagY);
	}
	gtk_widget_show(pChess->pLineup->pFlag);
}

void ShowSelect(Junqi *pJunqi, BoardChess *pChess)
{
	pJunqi->bSelect = 1;
	pJunqi->pSelect = pChess;


	ShowRectangle(pJunqi, pChess, RECTANGLE_WHITE);
}

void ShowPathArrow(Junqi *pJunqi, int iPath)
{
	GraphPath *p;
	int x,y;

	p=pJunqi->pPath[iPath];
	do
	{
		if(p->pChess->iDir&1)
		{
			x = p->pChess->xPos+8;
			y = p->pChess->yPos+9;
		}
		else
		{
			x = p->pChess->xPos+12;
			y = p->pChess->yPos+5;
		}
		gtk_fixed_put(GTK_FIXED(pJunqi->fixed), p->pArrow, x, y);
		gtk_widget_show(p->pArrow);

		p=p->pNext;
	}while( !p->isHead );
}

BoardChess *ShowBanner(Junqi *pJunqi, int iDir)
{
	BoardChess *pBanner;
	if( pJunqi->ChessPos[iDir][26].type==JUNQI )
	{
		pBanner = &pJunqi->ChessPos[iDir][26];
	}
	else
	{
		pBanner = &pJunqi->ChessPos[iDir][28];
	}

	for(int i=1; i<4; i++)
	{
		gtk_widget_destroy(pBanner->pLineup->pImage[i]);
		g_object_unref (pBanner->pLineup->pImage[i]);
	}
	SetChessImageType(pJunqi, iDir, pBanner->index, JUNQI);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed),
		     	  pBanner->pLineup->pImage[iDir],
		     	  pBanner->xPos,pBanner->yPos);
	gtk_widget_show(pBanner->pLineup->pImage[iDir]);
	SendSoundEvent(pJunqi, SHOW_FLAG);

	return pBanner;
}

void ClearChessFlag(Junqi *pJunqi, int iDir)
{
	int j;
	for(j=0;j<30;j++)
	{
		if(pJunqi->Lineup[iDir][j].pFlag)
		{
			gtk_widget_destroy(pJunqi->Lineup[iDir][j].pFlag);
			pJunqi->Lineup[iDir][j].pFlag = NULL;
		}
	}
}

void DestroyChessFlag(Junqi *pJunqi)
{
	int i,j;
	for(i=0;i<4;i++)
	{
		for(j=0;j<30;j++)
		{
			if(pJunqi->Lineup[i][j].pFlag)
			{
				gtk_widget_destroy(pJunqi->Lineup[i][j].pFlag);
				pJunqi->Lineup[i][j].pFlag = NULL;
			}
		}
	}
}

int aseertChess(BoardChess *pChess)
{
	int rc = 1;
	if( pChess && pChess->type!=NONE )
	{
		if( pChess!=pChess->pLineup->pChess )
		{
			rc = 0;
		}
	}
	return rc;
}

int aseertLineup(ChessLineup *pLineup)
{
	int rc = 1;
	if( pLineup && pLineup->type!=NONE && !pLineup->bDead )
	{
		if( pLineup!=pLineup->pChess->pLineup )
		{
			rc = 0;
		}
	}
	return rc;
}

int assertAllLineup(Junqi *pJunqi)
{
	int i,j;
	int rc = 1;
	for(i=0;i<4;i++)
	{
		for(j=0;j<30;j++)
		{
			rc = aseertLineup(&pJunqi->Lineup[i][j]);
			if(!rc) return 0;

		}
	}
	return rc;
}


void DestroyAllChess(Junqi *pJunqi, int iDir)
{
	int i,j;
	//BoardChess *pChess;
	ChessLineup *pLineup;

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->type!=NONE )
		{
			if( !pLineup->bDead )
			{
				pLineup->bDead = 1;
				pLineup->pChess->type = NONE;
			}
			for(j=1; j<4; j++)
			{
				gtk_widget_destroy(pLineup->pImage[j]);
				g_object_unref (pLineup->pImage[j]);
			}
		}
	}

	assert( assertAllLineup(pJunqi) );

	pJunqi->aInfo[iDir].bDead = 1;
	if( pJunqi->aInfo[(iDir+2)%4].bDead==1 )
	{
		if( !pJunqi->bReplay )
			pJunqi->bStart = 0;
	}

}

void AddDataToReplay(Junqi *pJunqi, const u8 *aBuf, int size)
{
	if( pJunqi->iReOfst+size>=PAGE_SIZE )
	{
		return;
	}
	memcpy(pJunqi->aReplay+pJunqi->iReOfst, aBuf, size);
	pJunqi->iReOfst += size;
}

void SetReplayData(Junqi *pJunqi, u8 *aBuf, int iOfst, int iAmt)
{
	memcpy(pJunqi->aReplay+iOfst, aBuf, iAmt);
}

void AddEventToReplay(Junqi *pJunqi, int event, int iDir)
{
	u8 aEvent[4];

	memset(aEvent, 0, 4);
	aEvent[0] = PLAY_EVENT;
	aEvent[1] = event;
	aEvent[2] = iDir;
	AddDataToReplay(pJunqi, aEvent, 4);
}

void AddLineupToReplay(Junqi *pJunqi)
{
	u8 aLineup[30];
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<30; j++)
		{
			aLineup[j] = pJunqi->Lineup[i][j].type;
		}
		AddDataToReplay(pJunqi, aLineup, 30);
	}
}

void AddMoveRecord(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst)
{
	u8 aMove[4];
	aMove[0] = pSrc->point.x;
	aMove[1] = pSrc->point.y;
	aMove[2] = pDst->point.x;
	aMove[3] = pDst->point.y;

	for(int i=0; i<4; i++)
	{
		assert( aMove[i]>=0 && aMove[i]<17 );
	}
	AddDataToReplay(pJunqi, aMove, 4);
}

/*
 * 根据比较结果，对棋子做相应处理
 * 此处代码具有很强的次序关系，修改时要小心
 */
void PlayResult(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst, int type)
{

	MoveResultData send_data;
	BoardChess *pBanner;
	int srcDir = 0;
	int dstDir = 0;

	srcDir = pSrc->pLineup->iDir;
	if( pDst->pLineup )
	{
		dstDir = pDst->pLineup->iDir;
	}

//	assert( aseertChess(pSrc) );
//	assert( aseertChess(pDst) );
//	assert( assertAllLineup(pJunqi) );


	memset(&send_data, 0, sizeof(MoveResultData));
	send_data.src[0] = pSrc->point.x;
	send_data.src[1] = pSrc->point.y;
	send_data.dst[0] = pDst->point.x;
	send_data.dst[1] = pDst->point.y;
	send_data.result = type;

	if( type==EAT || type==MOVE )
	{
		gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
					   pSrc->pLineup->pImage[pDst->iDir],
					   pDst->xPos,pDst->yPos);
		SendSoundEvent(pJunqi,MOVE);

	}
	if( type==EAT || type==BOMB )
	{
		assert(pDst->type!=NONE);
		gtk_widget_hide(pDst->pLineup->pImage[pDst->iDir]);
		if( pDst->pLineup->pFlag )
		{
			gtk_widget_destroy(pDst->pLineup->pFlag);
			pDst->pLineup->pFlag = NULL;
		}
		pDst->pLineup->bDead = 1;
		if( type==BOMB )
		{
			pSrc->pLineup->bDead = 1;
			pDst->type = NONE;
			SendSoundEvent(pJunqi,BOMB);
		}
		else
		{
			SendSoundEvent(pJunqi,EAT);
		}

		if( pDst->pLineup->type==JUNQI )
		{
			send_data.extra_info |= 0x01;
			DestroyAllChess(pJunqi, pDst->iDir);
			SendSoundEvent(pJunqi,DEAD);
			ClearChessFlag(pJunqi,pDst->iDir);
			HideJumpButton(pDst->iDir);
		}
	}

	if( type==EAT || type==MOVE )
	{
		pDst->type = pSrc->pLineup->type;
		pDst->pLineup = pSrc->pLineup;
		pDst->pLineup->pChess = pDst;
		if(pSrc->pLineup->pFlag)
		{
			MoveFlag(pJunqi,pDst,1);
		}
	}

	if( type==KILLED || type==BOMB )
	{
		if( type==KILLED )
		{
			pSrc->pLineup->bDead = 1;

			SendSoundEvent(pJunqi,KILLED);
			if( pSrc->pLineup->type==SILING )
			{
				send_data.extra_info |= 0x02;
				pBanner = ShowBanner(pJunqi, pSrc->pLineup->iDir);
				send_data.junqi_src[0] = pBanner->point.x;
				send_data.junqi_src[1] = pBanner->point.y;
			}
		}
		if(pSrc->pLineup->pFlag)
		{
			gtk_widget_hide(pSrc->pLineup->pFlag);
		}
		if( type==BOMB )
		{
			if( pSrc->pLineup->type==SILING )
			{
				send_data.extra_info |= 0x02;
				pBanner = ShowBanner(pJunqi, pSrc->pLineup->iDir);
				send_data.junqi_src[0] = pBanner->point.x;
				send_data.junqi_src[1] = pBanner->point.y;
			}
			if( pDst->pLineup->type==SILING )
			{
				send_data.extra_info |= 0x04;
				pBanner = ShowBanner(pJunqi, pDst->pLineup->iDir);
				send_data.junqi_dst[0] = pBanner->point.x;
				send_data.junqi_dst[1] = pBanner->point.y;
			}
		}
	}
	assert( pSrc->pLineup->type!=NONE );
	gtk_widget_hide(pSrc->pLineup->pImage[pSrc->iDir]);
	pSrc->type = NONE;
	if( type==EAT || type==MOVE )
	{
		gtk_widget_show(pDst->pLineup->pImage[pDst->iDir]);
	}
	ShowRectangle(pJunqi, pDst, RECTANGLE_RED);
	ShowPathArrow(pJunqi, 0);

	if(!pJunqi->bReplay)
	{
		AddMoveRecord(pJunqi, pSrc, pDst);
	}
	pJunqi->addr = pJunqi->addr_tmp[0];
	SendMoveResult(pJunqi, pSrc->pLineup->iDir, &send_data);
#ifndef NOT_DEBUG2
	pJunqi->addr = pJunqi->addr_tmp[1];
	SendMoveResult(pJunqi, pSrc->pLineup->iDir, &send_data);
#endif
	CheckIfDead(pJunqi, srcDir);
	CheckIfDead(pJunqi, dstDir);

//	assert( aseertChess(pSrc) );
//	assert( aseertChess(pDst) );
//	assert( aseertLineup(pSrc->pLineup) );
//	assert( aseertLineup(pDst->pLineup) );
//	assert( assertAllLineup(pJunqi) );
}


/*
 * 重新初始化标记图片
 * gtk的fixed有一个非常不友好的特性，后加入的控件会遮挡之前的控件
 * 目前没发现有什么方法把被遮挡的控件显示在最前面，所以只有
 * 先把被遮挡的控件销毁，再重新放到fixed里面
 */
void InitFlagImage(Junqi *pJunqi)
{
	GtkWidget *FlagImage = gtk_event_box_new();
	GtkWidget *image;
	char *imageName = "./res/flag1.png";
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), FlagImage, 0, 0);
	g_signal_connect(FlagImage, "button-press-event",
			G_CALLBACK(select_flag_event), pJunqi);
	image = gtk_image_new_from_file(imageName);
	gtk_container_add(GTK_CONTAINER(FlagImage),image);
	gtk_widget_show(image);
	pJunqi->flagObj.image = FlagImage;
}

/*
 * 右键点击时显示标记图片，不同的区域显示的相对位置有所不同
 */
void ShowFlagChess(Junqi *pJunqi, BoardChess *pChess)
{
	int x,y;
	switch(pChess->iDir)
	{
	case HOME:
	case LEFT:
		x = pChess->xPos+40;
		y = pChess->yPos-250;
		break;
	case OPPS:
		x = pChess->xPos+40;
		y = pChess->yPos+40;
		break;
	case RIGHT:
		x = pChess->xPos-195;
		y = pChess->yPos-250;
		break;
	default:
		break;
	}
	gtk_widget_destroy(pJunqi->flagObj.image);
	InitFlagImage(pJunqi);

	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
			       pJunqi->flagObj.image,
			       x, y);
	gtk_widget_show(pJunqi->flagObj.image);
	pJunqi->pSelect = pChess;
}

void SwapChess(Junqi *pJunqi, int i, int j, int dir)
{
	ChessLineup temp;
	BoardChess *pChess;

	memcpy(&temp, &pJunqi->Lineup[dir][i], sizeof(ChessLineup));
	memcpy(&pJunqi->Lineup[dir][i], &pJunqi->Lineup[dir][j], sizeof(ChessLineup));
	memcpy(&pJunqi->Lineup[dir][j], &temp, sizeof(ChessLineup));

	pJunqi->Lineup[dir][i].pChess = &pJunqi->ChessPos[dir][i];
	pJunqi->Lineup[dir][j].pChess = &pJunqi->ChessPos[dir][j];
	pJunqi->ChessPos[dir][i].type = pJunqi->ChessPos[dir][i].pLineup->type;
	pJunqi->ChessPos[dir][j].type = pJunqi->ChessPos[dir][j].pLineup->type;

	pChess = &pJunqi->ChessPos[dir][i];
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				   pChess->pLineup->pImage[pChess->iDir],
				   pChess->xPos,pChess->yPos);

	pChess = &pJunqi->ChessPos[dir][j];
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				   pChess->pLineup->pImage[pChess->iDir],
				   pChess->xPos,pChess->yPos);
}

void ChangeChess(Junqi *pJunqi, BoardChess *pChess)
{
	if(pChess->type!=NONE)
	{
		if(pChess->iDir==pJunqi->pSelect->iDir)
		{
			if( IsEnableChange(pJunqi->pSelect, pChess) )
			{
				SwapChess(pJunqi,pChess->index,
						  pJunqi->pSelect->index,
						  pChess->iDir);
				SendLineup(pJunqi, pChess->iDir);
			}
		}
	}
}

/*
 * 当鼠标点击棋盘时会触发该事件，移动棋子、标棋等
 */
void deal_mouse_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

	int x,y;
	BoardChess *pChess;

	Junqi *pJunqi = (Junqi *)data;
	x = event->x;
	y = event->y;

	pChess=GetChessPos(pJunqi,x,y);
	gtk_widget_hide(pJunqi->flagObj.image);
	if( pChess==NULL )
	{
		return;
	}
	//点击左键
    if( event->button==1 )
    {
    	if( !pJunqi->bSelect )
    	{
    		if( pChess->type==NONE )
    		{
    			return;
    		}
			if( pJunqi->bStart && pJunqi->eTurn!=pChess->pLineup->iDir )
			{
#if NOT_DEBUG1
				if( pJunqi->eTurn%2!=0 )
					return;
#endif
				if( !pJunqi->bReplay  )
					return;
			}
    		SendSoundEvent(pJunqi,SELECT);
    		ShowSelect(pJunqi, pChess);
    	}
    	else
    	{
    		gtk_widget_hide(pJunqi->whiteRectangle[0]);
    		gtk_widget_hide(pJunqi->whiteRectangle[1]);
    		pJunqi->bSelect = 0;
    		if(pChess==pJunqi->pSelect)
    		{
    			SendSoundEvent(pJunqi,SELECT);
    			pJunqi->pSelect = NULL;
    			return;
    		}
    		if(pJunqi->bStart)
    		{
    			if(pChess->type!=NONE)
    			{
    				//如果是自己或者是对家的棋
					if( (pChess->pLineup->iDir%2) == (pJunqi->pSelect->pLineup->iDir%2) )
					{
						if( pChess->pLineup->iDir == pJunqi->pSelect->pLineup->iDir )
						{
							ShowSelect(pJunqi, pChess);
						}
						return;
					}
    			}
    			if( IsEnableMove(pJunqi, pJunqi->pSelect, pChess, 1) )
    			{
            		gtk_widget_hide(pJunqi->redRectangle[0]);
            		gtk_widget_hide(pJunqi->redRectangle[1]);

    				int type;
    				type = CompareChess(pJunqi->pSelect, pChess);
    				PlayResult(pJunqi, pJunqi->pSelect, pChess, type);
    				ChessTurn(pJunqi);

    			}
    		}
    		else
    		{
    			ChangeChess(pJunqi, pChess);
    		}
    	}
    }
    else if( event->button==3 )
    {
    	//点击右键
    	if(pChess->type!=NONE)
    	{
    		ShowFlagChess(pJunqi, pChess);
    	}
    }
}

/*
 * 选择标记棋子，点击标记图片后会触发该事件
 */
void select_flag_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	int x,y;
	int iPosX;
	int iPosY;
	int iPos;
	GdkPixbuf *pixbuf;
	Junqi *pJunqi = (Junqi *)data;
	x = event->x;
	y = event->y;

	gtk_widget_hide(pJunqi->flagObj.image);
	if( x>=15 && x<=181)
	{
		if( y>=15 && y<=210 )
		{
			iPosX = (x-15)/44;
			if(y<=92)
			{
				iPosY = (y-15)/40;
			}
			else
			{
				iPosY = (y-18)/40;
			}
			if( (x<=15+iPosX*44+34) && (y<=15+iPosY*40+34) )
			{
				iPos = iPosX+iPosY*4;
				pixbuf = pJunqi->flagObj.paPixbuf[iPos];
				if(pJunqi->pSelect->pLineup->pFlag)
				{
					gtk_widget_destroy(pJunqi->pSelect->pLineup->pFlag);
				}
				pJunqi->pSelect->pLineup->pFlag = gtk_image_new_from_pixbuf(pixbuf);
				pJunqi->pSelect->pLineup->pixbuf = pixbuf;
				MoveFlag(pJunqi,pJunqi->pSelect,0);

			}
		}
		else if( (x>=49 && x<=146) && (y>=221 && y<=253) )
		{
			if(pJunqi->pSelect->pLineup->pFlag)
			{
				gtk_widget_destroy(pJunqi->pSelect->pLineup->pFlag);
				pJunqi->pSelect->pLineup->pFlag = NULL;
			}
		}
	}
}


void DestroySelectImage(Junqi *pJunqi)
{
	gtk_widget_destroy(pJunqi->whiteRectangle[0]);
	gtk_widget_destroy(pJunqi->whiteRectangle[1]);
	gtk_widget_destroy(pJunqi->redRectangle[0]);
	gtk_widget_destroy(pJunqi->redRectangle[1]);
}

void InitSelectImage(Junqi *pJunqi)
{
	pJunqi->whiteRectangle[0] = GetSelectImage(0, 0);
	pJunqi->whiteRectangle[1] = GetSelectImage(1, 0);
	pJunqi->redRectangle[0] = GetSelectImage(0, 1);
	pJunqi->redRectangle[1] = GetSelectImage(1, 1);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[1], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whiteRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whiteRectangle[1], 0, 0);

}

/*
 * 初始化标记棋子
 */
void InitFlagPixbuf(Junqi *pJunqi)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *temp;
	int x,y;
	char *imageName = "./res/flag1.png";
	int i;

	InitFlagImage(pJunqi);

	pixbuf = gdk_pixbuf_new_from_file(imageName, NULL);
	for(i=0; i<20; i++)
	{
		x = 15+(i%4)*43;
		if(i<8)
		{
			y = 16+(i/4)*38;
		}
		else
		{
			y = 25+(i/4)*37;
		}
		temp = gdk_pixbuf_new_subpixbuf(pixbuf,x,y,34,34);
		temp = gdk_pixbuf_scale_simple(temp,22,22,GDK_INTERP_BILINEAR);
		pJunqi->flagObj.paPixbuf[i] = temp;
	}
}

void InitArrowPixbuf(Junqi *pJunqi)
{
	GdkPixbuf *pixbuf;
	int i;
	char *imageName = "./res/arrow.png";
	GdkPixbuf *temp;
	pixbuf = gdk_pixbuf_new_from_file(imageName, NULL);
	for(i=0; i<8; i++)
	{
		temp = gdk_pixbuf_new_subpixbuf(pixbuf,8+30*i,8,14,14);
		pJunqi->paArrowPixbuf[i] = temp;
	}

}

void InitBoardItem(Junqi *pJunqi)
{
	//初始化选择框
	InitSelectImage(pJunqi);
	//初始化标记棋子
	InitFlagPixbuf(pJunqi);
	InitArrowPixbuf(pJunqi);
}

void ResetNineGrid(Junqi *pJunqi)
{
	int i;
	for(i=0; i<9; i++)
	{
		pJunqi->NineGrid[i].type = NONE;
	}
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
		pJunqi->NineGrid[i].point.x = 10-(i%3)*2;
		pJunqi->NineGrid[i].point.y = 6+(i/3)*2;
		pJunqi->NineGrid[i].isRailway = 1;
		pJunqi->NineGrid[i].isNineGrid = 1;
		CreatGraphVertex(pJunqi,&pJunqi->NineGrid[i]);

	}

}

void AddAdjNode(
		Junqi *pJunqi,
		BoardGraph *pVertex,
		int i,
		int j,
		u8 isNineGrid
		)
{
	u8 newFlag;
	AdjNode *pNew;
	AdjNode *pNode = pJunqi->aBoard[i][j].pAdjList;

	if( pNode!=NULL )
	{
		if(isNineGrid)
		{
			newFlag = pNode->pChess->isNineGrid;
		}
		else
		{
			newFlag = pNode->pChess->isRailway;
		}

		if( newFlag )
		{
			pNew = (AdjNode*)malloc(sizeof(AdjNode));
			pNew->pChess =pNode->pChess;
			pNew->pNext = pVertex->pAdjList->pNext;
			pVertex->pAdjList->pNext = pNew;
		}
	}
}

/*
 * 添加4个角上的铁路邻居
 */
void AddSpcNode(Junqi *pJunqi)
{
	BoardGraph *pVertex;
	int i,j;

	i = 10;
	j = 11;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, j, i, 0);
	pVertex = &pJunqi->aBoard[j][i];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 6;
	j = 11;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, i-1, j-1, 0);
	pVertex = &pJunqi->aBoard[i-1][j-1];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 6;
	j = 5;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, j, i, 0);
	pVertex = &pJunqi->aBoard[j][i];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 11;
	j = 6;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, i-1, j-1, 0);
	pVertex = &pJunqi->aBoard[i-1][j-1];
	AddAdjNode(pJunqi, pVertex, i, j, 0);
}

void InitBoardGraph(Junqi *pJunqi)
{
	int i,j;
	BoardGraph *pVertex;

	for(i=0; i<17; i++)
	{
		for(j=0; j<17; j++)
		{
			pVertex = &pJunqi->aBoard[i][j];
			if( pVertex->pAdjList!=NULL )
			{
				assert( pVertex->pAdjList->pChess!=NULL );
				if( pVertex->pAdjList->pChess->isRailway )
				{
					AddAdjNode(pJunqi, pVertex, i+1, j, 0);
					AddAdjNode(pJunqi, pVertex, i-1, j, 0);
					AddAdjNode(pJunqi, pVertex, i, j+1, 0);
					AddAdjNode(pJunqi, pVertex, i, j-1, 0);
					if( pVertex->pAdjList->pChess->isNineGrid )
					{
						AddAdjNode(pJunqi, pVertex, i+2, j, 1);
						AddAdjNode(pJunqi, pVertex, i-2, j, 1);
						AddAdjNode(pJunqi, pVertex, i, j+2, 1);
						AddAdjNode(pJunqi, pVertex, i, j-2, 1);
					}
				}
			}
		}
	}
	AddSpcNode(pJunqi);

#if 0//测试邻接表初始化是否正确
	i = 6;
	j = 8;
	pVertex = &pJunqi->aBoard[i][j];
	AdjNode *p;
	for(p = pVertex->pAdjList;p!=NULL;p=p->pNext)
	{
		log_b("%d %d",p->pChess->point.x,p->pChess->point.y);

	}
#endif
}

void InitCurveRail(Junqi *pJunqi)
{
	int i,j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<30; j++)
		{
			if(j%5==4)
			{
				pJunqi->ChessPos[i][j].eCurveRail = i+RAIL1;
				pJunqi->ChessPos[(i+1)%4][j-4].eCurveRail = i+RAIL1;
			}
		}
	}
	/////拐弯铁路测试//////////
#if 0
//	i = 6;
//	j = 11;
//	BoardGraph *pVertex = &pJunqi->aBoard[i+1][j+1];
//    log_a("rail %d",pVertex->pAdjList->pChess->eCurveRail);
#endif

}

/*
 * 初始化棋盘、棋子和标记棋等相关控件
 */
void CreatBoardChess(GtkWidget *window, Junqi *pJunqi)
{
	g_signal_connect(gtk_widget_get_parent(pJunqi->fixed), "button-press-event",
			G_CALLBACK(deal_mouse_press), pJunqi);

	LoadChessImage(pJunqi);
	for(int i=0; i<4; i++)
	{
		InitLineup(pJunqi,i);
		SetChess(pJunqi,i);
		for(int j=0; j<30; j++)
		{
			CreatGraphVertex(pJunqi,&pJunqi->ChessPos[i][j]);
		}
	}
	InitNineGrid(pJunqi);
	InitBoardItem(pJunqi);
	InitBoardGraph(pJunqi);
	InitCurveRail(pJunqi);
}

Junqi *JunqiOpen(void)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));
	pthread_mutex_init(&pJunqi->mutex, NULL);
	AddDataToReplay(pJunqi, aMagic, 4);
	return pJunqi;
}

void ConvertFilename(char *zName)
{
	while(*zName!='\0')
	{
		if(*zName=='\\')
		{
			*zName = '/';
		}
		zName++;
	}
}

void LoadLineup(Junqi *pJunqi, int iDir, u8 *chess)
{
	int i,j;

	for(i=0;i<30;i++)
	{
		if(pJunqi->ChessPos[iDir][i].type == NONE)
		{
			continue;
		}

		if( pJunqi->ChessPos[iDir][i].type != chess[i] )
		{
			for(j=i+1;j<30;j++)
			{
				if(chess[i]==pJunqi->ChessPos[iDir][j].type)
				{
					if( IsEnableChange(&pJunqi->ChessPos[iDir][j], &pJunqi->ChessPos[iDir][i]) )
					{
						SwapChess(pJunqi,i,j,iDir);
					}
					break;
				}
			}
		}
		assert( aseertChess(&pJunqi->ChessPos[iDir][i]) );
	}
	SendLineup(pJunqi, iDir);
}

void get_lineup_cb (GtkNativeDialog *dialog,
                  gint             response_id,
                  gpointer         user_data)
{
	char *name;
	Junqi *pJunqi = (Junqi *)user_data;
	GtkFileChooserNative *native = pJunqi->native;
	int fd;
	u8 aBuf[4096];
	Jql *pLineup;
	int iDir;

	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (native));
		ConvertFilename(name);
		fd = open(name, O_RDWR|O_CREAT, 0600);
		OsRead(fd, aBuf, 4096, 0);
		pLineup = (Jql*)(&aBuf[0]);
		if( memcmp(pLineup->aMagic, aMagic, 4)!=0 )
		{
			return;
			assert(0);
		}

		iDir = pJunqi->selectDir;

		LoadLineup(pJunqi, iDir, pLineup->chess);
	}

	gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
	g_object_unref (native);

}

void LoadReplayLineup(Junqi *pJunqi)
{
	u8 aChess[30];
	for(int i=0; i<4; i++)
	{
		memcpy(aChess, &pJunqi->aReplay[8+30*i], 30);
		LoadLineup(pJunqi, i, aChess);
	}

}

void OpenReplay(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data)
{
	char *name;
	Junqi *pJunqi = (Junqi *)user_data;
	GtkFileChooserNative *native = pJunqi->native;
	int fd;
	u8 aBuf[PAGE_SIZE];

	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (native));
		ConvertFilename(name);
		fd = open(name, O_RDWR|O_CREAT, 0600);
		OsRead(fd, aBuf, PAGE_SIZE, 0);
		if( memcmp(aBuf, aMagic, 4)==0 )
		{
			pJunqi->bReplay = 1;
			memcpy(pJunqi->aReplay, aBuf, PAGE_SIZE);
			ReSetChessBoard(pJunqi);
			LoadReplayLineup(pJunqi);
			ShowReplaySlider(pJunqi);
			int max_step = *(int *)(&pJunqi->aReplay[4]);
			gtk_adjustment_set_upper(pJunqi->slider_adj, max_step);

			pJunqi->bStart = 1;
			pJunqi->bStop = 1;
			pJunqi->iRpStep = 0;
		}
	}

	gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
	g_object_unref (native);
}

void ShowReplayStep(Junqi *pJunqi, u8 next_flag)
{
	int i;
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;


	ReSetChessBoard(pJunqi);

	for(i=0; i<pJunqi->iRpStep; i++)
	{
		pJunqi->sound_type = 0;

		if( *((u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i))==PLAY_EVENT )
		{
			int iDir = *((u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+2));
			if( *((u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+1))==SURRENDER_EVENT )
			{
				DestroyAllChess(pJunqi, iDir);
				SendSoundEvent(pJunqi,DEAD);
				ClearChessFlag(pJunqi,iDir);
			}
			else if( *((u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+1))==JUMP_EVENT )
			{
				IncJumpCnt(pJunqi, iDir);
				if( i==pJunqi->iRpStep-1 && next_flag)
					ShowDialogMessage(pJunqi, "跳过次数", pJunqi->aInfo[iDir].cntJump);
			}
			continue;
		}
		p1.x = *(u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i);
		p1.y = *(u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+1);
		p2.x = *(u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+2);
		p2.y = *(u8*)(pJunqi->aReplay+MOVE_OFFSET+4*i+3);

		assert( p1.x>=0 && p1.x<17 );
		assert( p1.y>=0 && p1.y<17 );
		assert( p2.x>=0 && p2.x<17 );
		assert( p2.x>=0 && p2.x<17 );
		pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;
		pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;

		if( IsEnableMove(pJunqi, pSrc, pDst, 1) )
		{
			gtk_widget_hide(pJunqi->redRectangle[0]);
			gtk_widget_hide(pJunqi->redRectangle[1]);

			int type;
			type = CompareChess(pSrc, pDst);
			PlayResult(pJunqi, pSrc, pDst, type);
			if( i==pJunqi->iRpStep-1 )
			{
				pJunqi->sound_type = pJunqi->sound_replay;
			}
		}
		else
		{
			assert(0);
		}
	}

}

void SaveLineup(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data)
{
	char *name;
	Junqi *pJunqi = (Junqi *)user_data;
	GtkFileChooserNative *native = pJunqi->native;
	int fd;
    char aBuf[50];
    int iDir;

	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (native));
		ConvertFilename(name);
		//strcat(name,".jql");

		fd = open(name, O_RDWR|O_CREAT, 0600);

		memcpy(aBuf,aLineupBuf,20);
		iDir = pJunqi->eLineupDir;
		for(int i=0; i<30; i++)
		{
			aBuf[20+i] = pJunqi->Lineup[iDir][i].type ;
		}

		OsWrite(fd, aBuf, sizeof(aBuf), 0);
	}

	gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
	g_object_unref (native);
}

void SaveReplay(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data)
{
	char *name;
	Junqi *pJunqi = (Junqi *)user_data;
	GtkFileChooserNative *native = pJunqi->native;
	int fd;
	int iTolalStep;

	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (native));
		ConvertFilename(name);
		fd = open(name, O_RDWR|O_CREAT, 0600);
		assert( pJunqi->iReOfst>=MOVE_OFFSET );
		iTolalStep = (pJunqi->iReOfst-MOVE_OFFSET)/4;
		SetReplayData(pJunqi, (u8*)&iTolalStep, 4, 4);
		OsWrite(fd, pJunqi->aReplay, pJunqi->iReOfst, 0);
	}
	gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
	g_object_unref (native);
}


void ReSetChessBoard(Junqi *pJunqi)
{
	assert( assertAllLineup(pJunqi) );
	for(int i=0; i<4; i++)
	{
		if( !pJunqi->aInfo[i].bDead )
		{
			DestroyAllChess(pJunqi, i);
		}
	}
	//先全部销毁，再重新初始化，否则棋子的关系会搞乱
	for(int i=0; i<4; i++)
	{
		SetChess(pJunqi,i);
	}

	ResetNineGrid(pJunqi);
	ClearPathArrow(pJunqi, 0);
	DestroySelectImage(pJunqi);
	InitSelectImage(pJunqi);
	//DestroyAllChess内部会把bDead置位，初始化重新清0
	memset(pJunqi->aInfo, 0, sizeof(pJunqi->aInfo));
}
