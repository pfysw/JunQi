/*
 * rule.h
 *
 *  Created on: Aug 14, 2018
 *      Author: Administrator
 */

#ifndef RULE_H_
#define RULE_H_

#include "junqi.h"

int IsEnableChange(BoardChess *pSrc, BoardChess *pDst);
int IsEnableMove(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst, u8 isShowPath);
int CompareChess(BoardChess *pSrc, BoardChess *pDst);
void ClearPathArrow(Junqi *pJunqi, int iPath);
void ChessTurn(Junqi *pJunqi);
void IncJumpCnt(Junqi *pJunqi, int iDir);
int CheckIfDead(Junqi *pJunqi, int iDir);

#endif /* RULE_H_ */
