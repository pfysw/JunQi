#ifndef JUNQI_H
#define JUNQI_H
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <windows.h>
#include <mmsystem.h>

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

enum ChessColor {ORANGE,PURPLE,GREEN,BLUE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	            LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
enum ChessDir {HOME,RIGHT,OPPS,LEFT};
enum SpcRail {RAIL1=1,RAIL2,RAIL3,RAIL4};
enum RailType {GONGB_RAIL,HORIZONTAL_RAIL,VERTICAL_RAIL,CURVE_RAIL};
enum CompareType {MOVE,EAT,BOMB,KILLED};

////////// test /////////////////////
#define log_a(format,...)   //printf(format"\n",## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)
#define log_b(format,...)  printf(format"\n",## __VA_ARGS__)
#define log_c(format,...)  //printf(format"\n",## __VA_ARGS__)
////////////////////////////////

#define MOVE_SOUND "./sound/move.wav"

const static u8 aMagic[4]={0x57,0x04,0,0};

typedef struct ChessLineup
{
	//表示棋子是哪家的棋
	enum ChessDir iDir;
	enum ChessType type;
	GtkWidget *pImage[4];
	GtkWidget *pFlag;
}ChessLineup;

typedef struct BoardPoint
{
	int x;
	int y;
}BoardPoint;

typedef struct BoardChess BoardChess;
struct BoardChess
{
	enum ChessType type;
	ChessLineup *pLineup;
	////下面为固定属性，不能改变///////////
	enum ChessDir iDir;
	enum SpcRail eCurveRail;
	int xPos;
	int yPos;
	int index;
	BoardPoint point;
	u8  isStronghold;
	u8  isCamp;
	u8  isRailway;
	u8  isNineGrid;

};

typedef struct FlagChess
{
	GdkPixbuf *paPixbuf[20];
	//GtkWidget *clearFlag;
	GtkWidget *image;
}FlagChess;

//邻接表adjacency list;
typedef struct VertexNode AdjNode;
struct VertexNode
{
	BoardChess *pChess;
	AdjNode *pNext;
};

typedef struct BoardGraph
{
	AdjNode *pAdjList;
	int passCnt;
}BoardGraph;

typedef struct GraphPath GraphPath;
struct GraphPath
{
	BoardChess *pChess;
	GtkWidget *pArrow;
	GraphPath *pNext;
	GraphPath *pPrev;
	u8 isHead;
};

typedef struct Junqi Junqi;
struct Junqi
{
	u8 eState;
	u8 bSelect;
	u8 bStart;
	enum ChessColor eColor;
	BoardChess *pSelect;
	GdkPixbuf *ChessImage[4];
	GdkPixbuf *Chess[4][14];
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	GtkWidget *fixed;
	//BoardChess ChessHome[4][30];
	BoardChess NineGrid[9];
	//棋盘是17*17，9宫格是5*5
	BoardGraph aBoard[17][17];
	//0：之前显示的路径，1：当前确定的最优路径，2：其他尝试的路径
	GraphPath  *pPath[3];
	int iPathLength;

	GtkWidget *whiteRectangle[2];
	GtkWidget *redRectangle[2];
	FlagChess flagObj;
	GdkPixbuf *paArrowPixbuf[8];

	gpointer data;
	GtkFileChooserNative *native;
	int selectDir;

};

typedef struct Jql
{
	u8 not_use[16];
	u8 aMagic[4];
	u8 chess[30];
}Jql;

void CreatBoardChess(GtkWidget *window, Junqi *pJunqi);
Junqi *JunqiOpen(void);
void get_lineup_cb (GtkNativeDialog *dialog,
                  gint             response_id,
                  gpointer         user_data);

/////////////////////////////////
int IsEnableChange(Junqi *pJunqi, BoardChess *pChess);
int IsEnableMove(Junqi *pJunqi, BoardChess *pDst);
int CompareChess(BoardChess *pSrc, BoardChess *pDst);

#endif
