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

void PrintLineup(Junqi *pJunqi)
{
    int i,j;
    ChessLineup *pLineup;
    int nChess = 30;

    for(j=0;j<4;j++)
    {
        log_a("dir %d:",j);
        for(i=0;i<nChess;i++)
        {
            pLineup = pJunqi->aChessPos[j*nChess+i].pLineup;
            if(pLineup){
                log_b("%d ",pLineup->type);
            }
            else{
                log_b("0 ");
            }
            if(i%5==4){
                log_a("");
            }
        }
    }
}

void CheckLineupInit(Skeleton *pSkl)
{
    Junqi *pJunqi = pSkl->pJunqi;
    PrintLineup(pJunqi);
}
