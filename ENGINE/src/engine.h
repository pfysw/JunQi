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
	CAMP_EVENT,
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
extern u8 aEventBit[100];

typedef struct MoveResult
{
    MoveResultData move;
    int percent;
    u8 flag;//标记是移动还是碰撞
}MoveResult;

typedef struct  BestMoveList  BestMoveList;
struct  BestMoveList
{
    MoveResult result[4];
    BestMoveList *pNext;
};

struct BestMove
{
    BestMoveList *pHead;
    BestMoveList *pNode;
    MoveList *pTest;
    u8 flag1; //判断是否已经搜索过
    u8 flag2; //标记这一层搜索无棋可走
    u8 mxPerFlag;
    u8 mxPerFlag1;
};

typedef struct ENGINE
{
	Junqi *pJunqi;
	//++++++++++++++
	//早期的代码，现在不用
	BoardChess *pCamp[2];
	BoardChess *pBomb[2];
	BoardChess *pEat[2];
	BoardChess *pMove[2];
	GraphPath *pPath[2];//pPath[0] 暂时不用
	u16 eventId;
    u8  eventFlag;
    //--------------------------
    BestMove aBestMove[30];
    BoardChess *pBest[2];
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

#endif /* ENGIN_H_ */
