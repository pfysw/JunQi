/*
 * skeleton.c
 *
 *  Created on: Nov 27, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include "skeleton.h"

Skeleton *SkeletonOpen(Junqi *pJunqi)
{
    Skeleton *pSkeleton = (Skeleton*)malloc(sizeof(Skeleton));
    memset(pSkeleton, 0, sizeof(Skeleton));
    pSkeleton->pJunqi = pJunqi;

    return pSkeleton;
}

void PrintBoardProp(Junqi *pJunqi,enum SklPrintType type)
{
    int i;
    int nChess = 30;
    ChessLineup *pLineup;
    BoardChess *pChess;

    for(i=0;i<129;i++)
    {
        pChess = &pJunqi->aChessPos[i];
        if(i%nChess==0){
            log_a("dir %d:",i/nChess);
        }
        switch(type){
        case SKL_LINEUP:
            pLineup = pChess->pLineup;
            if(pLineup){
                log_b("%d ",pLineup->type);
            }
            else{
                log_b("0 ");
            }
            break;
        case SKL_POS_PROP:
            log_b("%d ",pChess->prop);
            break;
        case SKL_POS_POINT:
            pChess = pJunqi->apBoard[pChess->point.x][pChess->point.y];
            log_b("(%d,%d) ",pChess->point.x,pChess->point.y);
            break;
        default:
            break;
        }

        if(i<120&&i%5==4){
            log_a("");
        }
    }
    log_a("");
}

void PrintRailwayLink(Junqi *pJunqi)
{
    int i;
    LinkNode *p;
    BoardChess *pChess;

    for(i=0;i<4;i++)
    {
        log_a("i %d",i);
        for(p=pJunqi->apRail[i]->pNext;!p->isHead;p=p->pNext)
        {
            pChess = (BoardChess *)p->pVal;
            log_a("(%d,%d) ",pChess->point.x,pChess->point.y);
        }

    }
}

void CheckChessInit(Skeleton *pSkl)
{
    Junqi *pJunqi = pSkl->pJunqi;
//    PrintBoardProp(pJunqi,SKL_LINEUP);
//    PrintBoardProp(pJunqi,SKL_POS_PROP);
//    PrintBoardProp(pJunqi,SKL_POS_POINT);
    PrintRailwayLink(pJunqi);
}
