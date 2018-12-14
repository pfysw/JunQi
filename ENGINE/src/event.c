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
