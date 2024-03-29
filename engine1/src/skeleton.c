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
    ChessLineup *pLineup;
    BoardChess *pChess;

    for(i=0;i<129;i++)
    {
        pChess = &pJunqi->aChessPos[i];
        if(i%CHESS_NUM==0){
            log_a("dir %d:",i/CHESS_NUM);
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

void PrintRailwayChess(Junqi *pJunqi)
{
    int i;
    RailInfo *pRail;
    BoardChess *pChess;
    LinkNode *p;

    for(i=0;i<129;i++)
    {
        pChess = &pJunqi->aChessPos[i];
        if(i%CHESS_NUM==0){
            log_a("dir %d:",i/CHESS_NUM);
        }
        log_a("i %d",i%30);
        if(pChess->prop!=RAILWAY){
            continue;
        }
        for(p=pChess->pRail->pNext;!p->isHead;p=p->pNext)
        {
            pRail = (RailInfo *)p->pVal;
            log_a("railway id %d",pRail->pHead->id);
            assert(pRail->pNode->pVal==pChess);
        }
    }
    log_a("");
}

void PrintRailwayLink(Junqi *pJunqi)
{
    int i;
    LinkNode *p;
    BoardChess *pChess;

    for(i=0;i<18;i++)
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
//    PrintRailwayLink(pJunqi);
    PrintRailwayChess(pJunqi);
}
