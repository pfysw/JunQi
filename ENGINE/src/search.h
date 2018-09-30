/*
 * search.h
 *
 *  Created on: Sep 20, 2018
 *      Author: Administrator
 */

#ifndef SEARCH_H_
#define SEARCH_H_

#include "junqi.h"

typedef struct PositionData PositionData;
struct PositionData
{
	BoardChess xSrcChess;
	BoardChess xDstChess;
	ChessLineup xSrcLineup;
	ChessLineup xDstLineup;
	u8 mx_type[30];
	PartyInfo enemyInfo;
	u8 junqi_type[2];
	u8 junqi_chess_type[2];
	BoardChess xJunqiChess[2];
	ChessLineup xJunqiLineup[2];
	u8 isSrcdead;
	u8 isDstDead;
};

typedef struct PositionList PositionList;
struct PositionList
{
	PositionData data;
	PositionList *pNext;
	PositionList *pPre;
	int value;
	u8 isHead;
};

void PopMoveFromStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove );

void PushMoveToStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove );


int AlphaBeta(
		Junqi *pJunqi,
		int depth,
		int alpha,
		int beta);
int TimeOut(Junqi *pJunqi);
u8 SendBestMove(Engine *pEngine);

#endif /* SEARCH_H_ */
