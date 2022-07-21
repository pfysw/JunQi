/*
 * rule.h
 *
 *  Created on: Aug 14, 2018
 *      Author: Administrator
 */

#ifndef RULE_H_
#define RULE_H_

#include "junqi.h"
#include "comm.h"

typedef struct FormInfo FormInfo;
struct FormInfo
{
    int nArrow;
    GtkWidget *apArrow[MAX_PAIR];
    int nLabel;
    GtkWidget *pathLabel[70];
};

int IsEnableChange(BoardChess *pSrc, BoardChess *pDst);
int IsEnableMove(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst, u8 isShowPath);
int CompareChess(BoardChess *pSrc, BoardChess *pDst);
void ClearPathArrow(Junqi *pJunqi, int iPath);
void ChessTurn(Junqi *pJunqi);
void IncJumpCnt(Junqi *pJunqi, int iDir);
int CheckIfDead(Junqi *pJunqi, int iDir);
void SendDeadEvent(Junqi *pJunqi, int iDir);
void GongbinPathTest(Junqi *pJunqi,BoardChess *pSrc,BoardChess *pDst);
void GongbinPathHide(Junqi *pJunqi);
void HideAllPathLabel(Junqi *pJunqi);
void ShowPathLabel(Junqi *pJunqi,int idx,int x,int y);
void ShowDebugPath(Junqi *pJunqi,PathDebug *pInfo);
void ShowPathInfo(Junqi *pJunqi, PathDebug *pInfo, int idx);

#endif /* RULE_H_ */
