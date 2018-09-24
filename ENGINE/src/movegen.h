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

void ClearMoveList(Junqi* pJunqi);
void GenerateMoveList(Junqi* pJunqi);
#endif /* MOVEGEN_H_ */
