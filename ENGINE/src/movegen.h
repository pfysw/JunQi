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
	u8 isHead;
	int value;

};

void ClearMoveList(MoveList *pHead);
MoveList *GenerateMoveList(Junqi* pJunqi, int iDir);
void SearchMovePath(
        Junqi* pJunqi,
        BoardChess *pSrc,
        int flag );
u8 IsDirectRail(
        Junqi *pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pDst );
void AddMoveToList(
    Junqi *pJunqi,
    BoardChess *pSrc,
    BoardChess *pDst);
#endif /* MOVEGEN_H_ */
