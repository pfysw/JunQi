#ifndef JUNQI_H
#define JUNQI_H
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <assert.h>
#include <windows.h>
#include <mmsystem.h>
#include "comm.h"
#include "type.h"



enum ChessColor {ORANGE,PURPLE,GREEN,BLUE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	            LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
enum ChessDir {HOME,RIGHT,OPPS,LEFT};
enum SpcRail {RAIL1=1,RAIL2,RAIL3,RAIL4};
enum RailType {GONGB_RAIL,HORIZONTAL_RAIL,VERTICAL_RAIL,CURVE_RAIL};
enum CompareType {MOVE=1,EAT,BOMB,KILLED,SELECT,SHOW_FLAG,DEAD,BEGIN,TIMER};


////////// test /////////////////////
#define log_a(format,...)   //printf(format"\n",## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)
#define log_b(format,...)  printf(format"\n",## __VA_ARGS__)
#define log_c(format,...)  //printf(format"\n",## __VA_ARGS__)

#if 0
static void memout(u8 *pdata,u8 len)
{
	int i;
	for(i=0;i<len;i++)
	{
		printf("%02X ",*(pdata+i));
		if((i+1)%8==0)
		{
			printf("\n");
		}
	}
	printf("\n");
}
#endif
////////////////////////////////

#define NOT_DEBUG1   0
//#define NOT_DEBUG2

#define MOVE_SOUND         "./sound/move.wav"
#define BOMB_SOUND         "./sound/bomb.wav"
#define EAT_SOUND          "./sound/eat.wav"
#define KILLED_SOUND       "./sound/killed.wav"
#define DEAD_SOUND         "./sound/dead.wav"
#define SHOW_FLAG_SOUND    "./sound/showflag.wav"
#define SELECT_SOUND       "./sound/select.wav"
#define BEGIN_SOUND        "./sound/begin.wav"
#define TIMER_SOUND        "./sound/timer.wav"

#define PAGE_SIZE 4096
#define MOVE_OFFSET (8+30*4)//4字节起始标志+4字节总步数+4家布阵

const static u8 aMagic[4]={0x57,0x04,0,0};

#define PLAY_EVENT 0xF5
#define JUMP_EVENT 0x00
#define SURRENDER_EVENT 0x01

typedef struct BoardChess BoardChess;
typedef struct ChessLineup
{
	//表示棋子是哪家的棋
	enum ChessDir iDir;
	enum ChessType type;
	GtkWidget *pImage[4];
	GtkWidget *pFlag;
	GdkPixbuf *pixbuf;
	BoardChess *pChess;
	u8 bDead;
}ChessLineup;

typedef struct BoardPoint
{
	int x;
	int y;
}BoardPoint;


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

typedef struct PartyInfo
{
	u8 bDead;
	u8 cntJump;
}PartyInfo;


struct Junqi
{
	u8 bReplay;
	u8 bSelect;
	u8 bStart;
	u8 bStop;
	u8 bAnalyse;
	enum ChessColor eColor;
	enum ChessDir eTurn;
	enum ChessDir eFirstTurn;
	BoardChess *pSelect;
	GdkPixbuf *ChessImage[4];
	GdkPixbuf *Chess[4][14];
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	GtkWidget *fixed;
	GtkWidget *window;
	GtkWidget *begin_button;
	GtkWidget *start_button;
	GtkWidget *step_fixed;
	GtkAdjustment *slider_adj;
	//GtkWidget *sound_obj;
	enum CompareType sound_type;
	enum CompareType sound_replay;
	//BoardChess ChessHome[4][30];
	BoardChess NineGrid[9];
	//棋盘是17*17，9宫格是5*5
	BoardGraph aBoard[17][17];
	//0：之前显示的路径，1：当前确定的最优路径，2：其他尝试的路径
	GraphPath  *pPath[3];
	u8 szPath;
	u8 szPathForSound;
	u8 aReplay[4096];
	int iReOfst;
	int iRpStep;
	PartyInfo aInfo[4];

	GtkWidget *whiteRectangle[2];
	GtkWidget *redRectangle[2];
	FlagChess flagObj;
	GdkPixbuf *paArrowPixbuf[8];
	char label_str[100];
	GtkWidget *pTimeLabel;

	gpointer data;
	GtkFileChooserNative *native;
	int selectDir;
	enum ChessDir eLineupDir;

	struct sockaddr_in addr;
	struct sockaddr_in addr_tmp[2];
	int socket_fd;
	GtkWidget *comm;
	u8 *pCommData;
	pthread_mutex_t mutex;
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
void SaveReplay(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data);
void OpenReplay(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data);
void SaveLineup(GtkNativeDialog *dialog,
        gint             response_id,
        gpointer         user_data);
void SendSoundEvent(Junqi *pJunqi, enum CompareType type);
void ReSetChessBoard(Junqi *pJunqi);
void DestroyAllChess(Junqi *pJunqi, int iDir);
void AddLineupToReplay(Junqi *pJunqi);
void AddEventToReplay(Junqi *pJunqi, int event, int iDir);
void ShowReplayStep(Junqi *pJunqi, u8 next_flag);
int CompareChess(BoardChess *pSrc, BoardChess *pDst);
void PlayResult(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst, int type);
void ChessTurn(Junqi *pJunqi);
void LoadLineup(Junqi *pJunqi, int iDir, u8 *chess);
void DestroyChessFlag(Junqi *pJunqi);
void MoveFlag(Junqi *pJunqi, BoardChess *pChess, int isInit);
void ClearChessFlag(Junqi *pJunqi, int iDir);
int aseertLineup(ChessLineup *pLineup);

#endif
