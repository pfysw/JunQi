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


extern int free_cnt;
extern int malloc_cnt;

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
	u8 nEat;
	u8 nBigEat;
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
	u8  isSapperPath;
	u8  pathCnt;
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
	u8 cnt[28];
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
    int deadValue;
	u8 bDead;
	u8 cntJump;
	u8 bShowFlag;
	u8 aTypeNum[14];
	u8 aLiveTypeSum[14];//大于某个级别的明棋总和
	u8 aLiveAllNum[14];//大于某个级别的明棋和暗棋总和
	u8 nMayBomb;
	u8 nMayLand;
	u8 nMayBombLand;
}PartyInfo;

typedef struct JunqiPathList JunqiPathList;
struct JunqiPathList
{
    //第一次初始化后就不会修改
    u8 index;
    u8 nChess;//暂时不用
    u8 nMayLand;//暂时不用
    u8 iDir;
    JunqiPathList *pNext[2];
};

typedef struct JunqiPath JunqiPath;
struct JunqiPath
{
    u8 index;
    u8 nChess;
    u8 nMayLand;
    u8 iDir;
    u8 isHead;
    JunqiPath *pNext;
    JunqiPath *pPre;
};

//typedef struct JunqiPathData
//{
//    u8 nChess;
//    u8 nMayLand;
//    JunqiPathList *pHead;
//}JunqiPathData;


typedef struct MoveNodeSlot MoveNodeSlot;
struct MoveNodeSlot
{
    MoveNodeSlot *pNext;
    MoveList node;
};

typedef struct MoveNodeMem
{
    MoveNodeSlot *pStart;
    MoveNodeSlot *pFree;
}MoveNodeMem;



struct Junqi
{
	u8 bStart;
	u8 bStop;
	u8 bGo;
	u8 bSearch;
	u8 bMove;
	u8 findMoveFlag;
	u8 nNoEat;
	u8 bAnalyse;
	enum ChessDir eTurn;
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	BoardChess NineGrid[9];
	//棋盘是17*17，9宫格是5*5
	BoardGraph aBoard[17][17];

	PartyInfo aInfo[4];
	Engine *pEngine;
	Junqi *pJunqiBase;
	MoveList *pMoveList;
	JunqiPathList *paPath[4][2];//0：从index0开始，1:从index4开始
	void *pThreadMem;
	MoveNodeMem mem_pool;

	int nRpStep;
	int iRpOfst;
	int begin_time;
	int test_time[2];
	int test_gen_num;
	int test_num;
	int searche_num[2];
	int iKey;
	SearchType eSearchType;
	u8 begin_flag;
	u8 cntSearch;
	u8 cnt;
	u8 myTurn;
	int malloc_cnt;
	int free_cnt;
	MoveHash **paHash;

	struct sockaddr_in addr;
	int socket_fd;
	mqd_t qid;
	mqd_t search_qid;
	mqd_t print_qid;
	pthread_mutex_t mutex;
	pthread_mutex_t search_mutex;
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
void ChessBoardCopy(Junqi *pJunqi);
void ClearAdjNode(Junqi *pJunqi);

void PrognosisChess(
        Junqi *pJunqi,
        int iDir);
void AdjustMaxType(Junqi *pJunqi, int iDir);

#endif /* JUNQI_H_ */
