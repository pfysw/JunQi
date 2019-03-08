/*
 * movegen.h
 *
 *  Created on: Sep 21, 2018
 *      Author: Administrator
 */

#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include "comm.h"

extern char *aTypeName[14];
typedef struct MoveList MoveList;
struct MoveList
{
	MoveResultData move;
	u16 percent;
	MoveList *pNext;
	MoveList *pPre;
	int iKey;
	u8 keyFlag;
	u8 isHead;
	int value;

};

typedef struct MoveStack MoveStack;
struct MoveStack
{
    u8 bSetNotBomb;
    u8 bSetMaxType;
    u8 tempType;
    u8 isNotBomb;
};

void ClearMoveList(Junqi *pJunqi, MoveList *pHead);
MoveList *GenerateMoveList(Junqi* pJunqi, int iDir);
void SearchMovePath(
        Junqi* pJunqi,
        BoardChess *pSr );
u8 IsDirectRail(
        Junqi *pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pPre,
        BoardGraph *pDst );
void AddMoveToList(
    Junqi *pJunqi,
    BoardChess *pSrc,
    BoardChess *pDst,
    AlphaBetaData *pData);
#endif /* MOVEGEN_H_ */
