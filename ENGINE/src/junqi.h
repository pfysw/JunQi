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
#include <assert.h>
#include "engine.h"
#include "utiliy.h"
#include "movegen.h"

enum ChessColor {ORANGE,PURPLE,GREEN,BLUE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	            LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
enum ChessDir {HOME,RIGHT,OPPS,LEFT};
enum SpcRail {RAIL1=1,RAIL2,RAIL3,RAIL4};
enum RailType {GONGB_RAIL,HORIZONTAL_RAIL,VERTICAL_RAIL,CURVE_RAIL};
enum CompareType {MOVE=1,EAT,BOMB,KILLED,SELECT,SHOW_FLAG,DEAD,BEGIN,TIMER};


#define PLAY_EVENT 0xF5
#define JUMP_EVENT 0x00
#define SURRENDER_EVENT 0x01

#define MOVE_OFFSET (8+30*4)//4字节起始标志+4字节总步数+4家布阵

typedef struct BoardChess BoardChess;
typedef struct ChessLineup
{
	enum ChessDir iDir;//表示棋子是哪家的棋
	//要注意子力越大，type值越小，见ChessType定义
	enum ChessType type;//如果是地方的棋，则表示最小可能
	enum ChessType mx_type;//预测敌方棋的最大可能
	BoardChess *pChess;
	u8 bDead;
	u8 bBomb;
	u8 index;
	u8 isNotLand;
	u8 isNotBomb;
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
	int pathCnt;
	u8  pathFlag;
	u8  sameFlag;
	////下面为固定属性，不能改变///////////
	enum SpcRail eCurveRail;
	enum ChessDir iDir;
	int index;
	BoardPoint point;
	u8  isStronghold;
	u8  isCamp;
	u8  isRailway;
	u8  isNineGrid;
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
	u8 bShowFlag;
	u8 aTypeNum[14];
}PartyInfo;

struct Junqi
{
	u8 bStart;
	u8 bStop;
	u8 bGo;
	u8 bSearch;
	u8 bMove;
	enum ChessDir eTurn;
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	BoardChess NineGrid[9];
	//棋盘是17*17，9宫格是5*5
	BoardGraph aBoard[17][17];

	PartyInfo aInfo[4];
	Engine *pEngine;
	MoveList *pMoveList;

	int nRpStep;
	int iRpOfst;
	int begin_time;
	int test_num;

	struct sockaddr_in addr;
	int socket_fd;
	mqd_t qid;
	mqd_t print_qid;
	pthread_mutex_t mutex;
};

Junqi *JunqiOpen(void);
void InitChess(Junqi* pJunqi, u8 *data);
void DestroyAllChess(Junqi *pJunqi, int iDir);
void IncJumpCnt(Junqi *pJunqi, int iDir);
void ChessTurn(Junqi *pJunqi);
void PlayResult(
		Junqi *pJunqi,
		BoardChess *pSrc,
		BoardChess *pDst,
		MoveResultData* pResult
		);
void InitBoard(Junqi* pJunqi);
void InitLineup(Junqi* pJunqi, u8 *data, u8 isInit);

#endif /* JUNQI_H_ */
