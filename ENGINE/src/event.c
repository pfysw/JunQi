/*
 * event.c
 *
 *  Created on: Aug 27, 2018
 *      Author: Administrator
 */
#include "event.h"
#include "path.h"

u8 aEventBit[100]={0};

char *aTypeName[14] =
{
	"none","dark","junqi","dilei","zhadan","siling","junzh",
	"shizh","lvzh","tuanzh","yingzh","lianzh","paizh","gongb"
};

#if 1
void CheckJunqiEvent(Engine *pEngine)
{
	u8 isMove = 0;
	int i;
	Junqi *pJunqi = pEngine->pJunqi;
	ChessLineup *pLineup;

	if( ENGINE_DIR%2!=pJunqi->eTurn%2 )
	{
		return;
	}

	CLEARBIT(aEventBit, MOVE_EVENT);
	CLEARBIT(aEventBit, JUNQI_EVENT);

	ClearPathCnt(pJunqi);
	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[pJunqi->eTurn][i];
		if( pLineup->bDead || pLineup->type==NONE ||
			pLineup->type==DILEI || pLineup->type==JUNQI )
		{
			continue;
		}
		isMove = CanMovetoJunqi(pEngine, pLineup->pChess);
		(void)isMove;
	}

}

u8 ProJunqiEvent(Engine *pEngine)
{
	Junqi *pJunqi = pEngine->pJunqi;

	assert( pEngine->pPath[1]==NULL );
	assert( IsEnableMove(pJunqi, pEngine->pMove[0], pEngine->pMove[1]) );
	SendMove(pJunqi, pEngine->pMove[0], pEngine->pMove[1]);

	CLEARBIT(aEventBit, MOVE_EVENT);
	CLEARBIT(aEventBit, JUNQI_EVENT);

	return 1;
}


void ChecAttackEvent(Engine *pEngine)
{
    int iDir;
    Junqi *pJunqi = pEngine->pJunqi;
    JunqiPath *pPath;

    if( ENGINE_DIR%2!=pJunqi->eTurn%2 )
    {
        return;
    }
    iDir = pJunqi->eTurn;
    ClearJunqiPath(pEngine, 0);
    GetJunqiPath(pEngine,pJunqi->paPath[(iDir+1)&3][1]);
    //GetJunqiPath(pEngine,pJunqi->paPath[(iDir+3)&3][1]);
    pPath = pEngine->pJunqiPath[0];
    log_a("dir %d nLand %d nChess %d",pPath->iDir,pPath->nMayLand,pPath->nChess);
    for( ; ; pPath=pPath->pNext)
    {
        log_a("index %d",pPath->index);
        if( pPath->pNext->isHead )
        {
            break;
        }
    }
}

//以上代码为开发过程中的历史代码，不再使用
//---------------------------------------------------------
#endif

//敌方有只有一个子时，可能计算出该子是炸弹
//如果我方只有大子，可能不敢去面对
void CheckLiveChessNum(Engine *pEngine)
{
    Junqi *pJunqi = pEngine->pJunqi;
    ChessLineup *pLineup;
    ChessLineup *pLive;
    int nLive = 0;
    int i,j;

    for(i=0; i<4; i++)
    {
        nLive = 0;
        if( !pJunqi->aInfo[i].bDead )
        {
            for(j=0; j<30; j++)
            {
                pLineup = &pJunqi->Lineup[i][j];
                if( pLineup->bDead || pLineup->type==NONE  )
                {
                    continue;
                }
                if( pLineup->pChess->isStronghold )
                {
                    continue;
                }
                if( i%2==ENGINE_DIR )
                {
                    if( pLineup->type!=DILEI )
                    {
                        nLive++;
                    }
                }
                else if( pLineup->isNotLand )
                {
                    nLive++;
                    pLive = pLineup;
                }
            }
            if( 1==nLive && i%2!=ENGINE_DIR )
            {
                pLive->isNotBomb = 1;
            }

        }
    }

}

int CalCampAroundNum(Junqi *pJunqi, BoardChess *pSrc)
{
    int i;
    int x,y;
    BoardChess *pNbr;
    int num = 0;

    for(i=0; i<9; i++)
    {
        if( i==4 ) continue;
        x = pSrc->point.x+1-i%3;
        y = pSrc->point.y+i/3-1;

        if( pJunqi->aBoard[x][y].pAdjList )
        {
            pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;
            if( pNbr->type!=NONE && !pNbr->isCamp )
            {
                if( pNbr->pLineup->iDir%2==pNbr->iDir%2 )
                {
                    num++;
                }
            }
        }
    }

    return num;
}

void CheckEmptyCamp(Engine *pEngine)
{
    Junqi *pJunqi = pEngine->pJunqi;
    BoardChess *pCamp;
    u8 aIndex[5] = {6,8,12,16,18};
    int i,j;

    for(i=0; i<4; i++)
    {
        if( !pJunqi->aInfo[i].bDead )
        {
            for(j=0; j<5; j++)
            {

                pCamp = &pJunqi->ChessPos[i][aIndex[j]];
                if( !CalCampAroundNum(pJunqi,pCamp) )
                {
                    pCamp->isCamp = 2;
                }
                else
                {
                    pCamp->isCamp = 1;
                }
                if( i%2==ENGINE_DIR && j>0 )
                {
                    break;
                }
            }
        }
    }
}

void SetMaxDepth(Engine *pEngine)
{
    if( pEngine->gInfo.timeSearch<2 )
    {
        pEngine->gInfo.mxDepth++;
    }
    else if( pEngine->gInfo.timeSearch>6 )//6秒
    {
        pEngine->gInfo.mxDepth--;
    }

    if( pEngine->gInfo.mxDepth<4 )
    {
        pEngine->gInfo.mxDepth = 4;
    }
    if( pEngine->gInfo.mxDepth>8 )
    {
        pEngine->gInfo.mxDepth = 8;
    }
}
void CheckBebombNum(Engine *pEngine)
{
    Junqi *pJunqi = pEngine->pJunqi;
    ChessLineup *pLineup;
    int i,j;

    for(i=0; i<4; i++)
    {
        if( !pJunqi->aInfo[i].bDead &&
                pJunqi->aInfo[i].nExchange>1 )
        {
            for(j=0;  j<30; j++)
            {
                pLineup = &pJunqi->Lineup[i][j];
                if( pLineup->bDead || pLineup->type==NONE  )
                {
                    continue;
                }
                if( pLineup->pChess->isStronghold )
                {
                    continue;
                }
                if( i%2!=ENGINE_DIR )
                {
                    if( 2==pLineup->isMayBomb )
                    {
                        pLineup->isMayBomb = 0;
                    }
                }
            }
        }
    }
}



void CheckGLobalInfo(Engine *pEngine)
{
   // Junqi *pJunqi = pEngine->pJunqi;
    CheckLiveChessNum(pEngine);
    CheckEmptyCamp(pEngine);
    CheckBebombNum(pEngine);
}
