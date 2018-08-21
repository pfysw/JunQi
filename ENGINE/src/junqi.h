/*
 * junqi.h
 *
 *  Created on: Aug 17, 2018
 *      Author: Administrator
 */

#ifndef JUNQI_H_
#define JUNQI_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "comm.h"
#include <assert.h>

enum ChessColor {ORANGE,PURPLE,GREEN,BLUE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	            LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
enum ChessDir {HOME,RIGHT,OPPS,LEFT};
enum SpcRail {RAIL1=1,RAIL2,RAIL3,RAIL4};
enum RailType {GONGB_RAIL,HORIZONTAL_RAIL,VERTICAL_RAIL,CURVE_RAIL};
enum CompareType {MOVE=1,EAT,BOMB,KILLED,SELECT,SHOW_FLAG,DEAD,BEGIN,TIMER};

////////// test /////////////////////
#define log_a(format,...)   printf(format"\n",## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)
#define log_b(format,...)  printf(format"\n",## __VA_ARGS__)
#define log_c(format,...)  //printf(format"\n",## __VA_ARGS__)

void memout(u8 *pdata,u8 len);

#define PLAY_EVENT 0xF5
#define JUMP_EVENT 0x00
#define SURRENDER_EVENT 0x01

typedef struct BoardChess BoardChess;
typedef struct ChessLineup
{
	//表示棋子是哪家的棋
	enum ChessDir iDir;
	enum ChessType type;
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
	enum SpcRail eCurveRail;
	int index;
	BoardPoint point;
	u8  isStronghold;
	u8  isCamp;
	u8  isRailway;
	u8  isNineGrid;
    u8 ss1;
};

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
	u8 bStart;
	u8 bStop;
	enum ChessDir eTurn;
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	BoardChess NineGrid[9];
	//棋盘是17*17，9宫格是5*5
	BoardGraph aBoard[17][17];

	PartyInfo aInfo[4];

	struct sockaddr_in addr;
	int socket_fd;
	mqd_t qid;
};

Junqi *JunqiOpen(void);
void InitChess(Junqi* pJunqi, u8 *data);
void DestroyAllChess(Junqi *pJunqi, int iDir);
void IncJumpCnt(Junqi *pJunqi, int iDir);
void ChessTurn(Junqi *pJunqi);
int IsEnableMove(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst);
void PlayResult(
		Junqi *pJunqi,
		BoardChess *pSrc,
		BoardChess *pDst,
		MoveResultData* pResult
		);
void InitBoard(Junqi* pJunqi);
void InitLineup(Junqi* pJunqi, u8 *data, u8 isInit);

#endif /* JUNQI_H_ */
