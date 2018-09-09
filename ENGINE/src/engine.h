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

enum MoveEvent{
	CAMP_EVENT,
	MOVE_EVENT,
	GONGB_EVENT,
	DARK_EVENT,
	EAT_EVENT,
	JUNQI_EVENT,
	BOMB_EVENT
};

#define TEST

#ifdef  TEST
#define ENGINE_DIR   0
#else
#define ENGINE_DIR   1
#endif

extern u8 aEventBit[100];

typedef struct ENGINE
{
	Junqi *pJunqi;
	BoardChess *pCamp[2];
	BoardChess *pBomb[2];
	BoardChess *pEat[2];
	BoardChess *pMove[2];
	GraphPath *pPath[2];//pPath[0] 暂时不用
	u16 eventId;
    u8  eventFlag;
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
