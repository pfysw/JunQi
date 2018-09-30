/*
 * movegen.h
 *
 *  Created on: Sep 21, 2018
 *      Author: Administrator
 */

#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include "comm.h"

typedef struct MoveList MoveList;
struct MoveList
{
	MoveResultData move;
	MoveList *pNext;
	MoveList *pPre;
	int value;
	u8 isHead;
};

void ClearMoveList(MoveList *pHead);
MoveList *GenerateMoveList(Junqi* pJunqi, int iDir);

#endif /* MOVEGEN_H_ */
