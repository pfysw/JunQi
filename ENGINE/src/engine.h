/*
 * engin.h
 *
 *  Created on: Aug 18, 2018
 *      Author: Administrator
 */

#ifndef ENGIN_H_
#define ENGIN_H_
#include "type.h"
#include <unistd.h>
#include <fcntl.h>
#include "evaluate.h"
#include "comm.h"
#include "movegen.h"

enum MoveEvent{
	MOVE_EVENT,
	GONGB_EVENT,
	DARK_EVENT,
	EAT_EVENT,
	JUNQI_EVENT,
	BOMB_EVENT
};

//#define TEST

#ifdef  TEST
#define ENGINE_DIR   0
#else
#define ENGINE_DIR   1
#endif

//#define EVENT_TEST

#define INFINITY 10000
#define RESULT_NUM 6
#define BEST_LIST_NUM 20
#define RECORD_LINEUP__NUM 20
#define MEM_POOL_LENGTH 60000

extern u8 aEventBit[100];

typedef struct MoveResult
{
    MoveResultData move;
    int value;
    int percent;
    u8 flag;//标记是移动还是碰撞
}MoveResult;

typedef struct  BestMoveList  BestMoveList;
struct  BestMoveList
{
    MoveResult result[RESULT_NUM];
    BestMoveList *pNext;
    u8 isMove;
    u8 index;
};

struct BestMove
{
    BestMoveList *pHead;
    BestMoveList *pNode;
    MoveList *pTest;
    int pathValue;
    u8 flag1; //判断是否已经搜索过
    u8 flag2; //标记这一层搜索不是无棋可走
    u8 mxPerFlag;
    u8 mxPerFlag1;
};

typedef struct MoveValue
{
    u8 src[2];
    u8 dst[2];
    int value;
}MoveValue;

typedef struct MoveSort MoveSort;
struct MoveSort
{
    BestMoveList *pHead;
    MoveSort *pNext;
    MoveSort *pPre;
    int aValue[6][20];
    u8 isSetValue[6][20];
    u8 isHead;
    u8 nNode;
    u8 rank;
};

typedef struct SearchMsg SearchMsg;
struct SearchMsg
{
    SearchType type;
    MoveSort *pNode;
    u8 deepDepth;
    u8 deepType;
};

typedef struct RecordLineup RecordLineup;
struct RecordLineup
{

    ChessLineup *pLineup;
    int iRpOfst;
    u8 isRecord;
};

typedef struct GLOBAL_INFO GLOBAL_INFO;
struct GLOBAL_INFO
{
    int timeStamp;
    int timeSearch;
    int mxDepth;
    u8 isSetShowFlag;
    u8 showIndex;
    u8 isOnlyOneChess;
    ChessLineup *pOnly;
    JunqiInfo *pDarkinfo;
};

typedef struct ENGINE
{
	Junqi *pJunqi;
	//++++++++++++++
	//早期的代码，现在不用
	BoardChess *pMove[2];
	GraphPath *pPath[2];//pPath[0] 暂时不用
	u16 eventId;
    u8  eventFlag;
    //--------------------------
    //MoveValue aMoveArray[100];
    u8 iHashOfst;
    RecordLineup aRecord[RECORD_LINEUP__NUM];
    GLOBAL_INFO gInfo;
    MoveSort **ppMoveSort;
    JunqiPath *pJunqiPath[2];
    BestMove aBestMove[BEST_LIST_NUM];
    MoveList *pFirstMove;
    MoveList *pDebugMove[10];
    BoardChess *pBest[2];
    u8 aSendBest[4];
    u8 eatInList;
    u8 eatIndex;
    PositionList *pPos;
    Value_Parameter valPara;
}Engine;

typedef struct EventHandle
{
	u8 (*xEventFun)(Engine *pEngine);
	u16  eventId;
}EventHandle;


pthread_t CreatEngineThread(Junqi* pJunqi);
void SendEvent(Junqi* pJunqi, int iDir, u8 event);
Engine *OpneEnigne(Junqi *pJunqi);
void CloseEngine(Engine *pEngine);
void ReSearchInDeep(Junqi* pJunqi, MoveSort *pNode, int depth);
void ReduceMaxDepth(Engine *pEngine);

#endif /* ENGIN_H_ */
