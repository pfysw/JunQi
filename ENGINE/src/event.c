/*
 * event.c
 *
 *  Created on: Aug 27, 2018
 *      Author: Administrator
 */
#include "event.h"

u8 aEventBit[100]={0};


u8 MoveInCamp(Engine *pEngine, BoardChess *pCamp)
{
    u8 isMove = 0;
    Junqi *pJunqi = pEngine->pJunqi;
    BoardChess *pNbr;
    int i;
    int x,y;

	for(i=0; i<9; i++)
	{
		if( i==4 ) continue;
		x = pCamp->point.x+1-i%3;
		y = pCamp->point.y+i/3-1;
		if( pJunqi->aBoard[x][y].pAdjList )
		{
			pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;
			if( pNbr->pLineup!=NULL &&
				pNbr->pLineup->iDir==pJunqi->eTurn &&
				!pNbr->pLineup->pChess->isCamp )
			{
				if( IsEnableMove(pJunqi, pNbr, pCamp) )
				{
					log_a("event camp0 %d %d",pCamp->point.x,
							pEngine->pCamp[0]->point.y);
					log_a("nbr %d %d",pNbr->point.x,pNbr->point.y);
					SendMove(pJunqi, pNbr, pCamp);
					isMove = 1;

					break;
				}
			}
		}
	}

    return isMove;
}

u8 ComeInCamp(Engine *pEngine)
{
    u8 isMove;

    isMove = MoveInCamp(pEngine, pEngine->pCamp[0]);
    if( isMove )
    {
		pEngine->pCamp[0] = pEngine->pCamp[1];
		if( pEngine->pCamp[1]==NULL )
		{
			CLEARBIT(aEventBit, CAMP_EVENT);
		}
		else
		{
			pEngine->pCamp[1] = NULL;
		}
    }
    else if( pEngine->pCamp[1] )
    {
    	isMove = MoveInCamp(pEngine, pEngine->pCamp[1]);
    	if( isMove )
    	{
    		pEngine->pCamp[1]=NULL;
    	}
    }

    return isMove;
}

void CheckCampEvent(Engine *pEngine,BoardChess *pChess)
{
	int i;
	int x,y;
	BoardChess *pNbr;
	Junqi *pJunqi = pEngine->pJunqi;

	if( ENGINE_DIR%2!=pChess->iDir%2 )
	{
		return;
	}

	for(i=0; i<9; i++)
	{
		if( i==4 ) continue;
		x = pChess->point.x+1-i%3;
		y = pChess->point.y+i/3-1;
		if(x<0||x>16||y<0||y>16) continue;

		if( pJunqi->aBoard[x][y].pAdjList )
		{

			pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;

			if( pNbr->isCamp && pNbr->type==NONE )
			{
					if( TESTBIT(aEventBit, CAMP_EVENT) )
					{
						log_a("camp1 %d %d",pEngine->pCamp[0]->point.x,
								pEngine->pCamp[0]->point.y);
						pEngine->pCamp[1] = pEngine->pCamp[0];
					}
					log_a("camp0 %d %d",pNbr->point.x,pNbr->point.y);
					pEngine->pCamp[0] = pNbr;
					SETBIT(aEventBit, CAMP_EVENT);
			}
		}
	}
}

u8 CanBombChess(Engine *pEngine, BoardChess *pBomb, int iDir)
{
	u8 isBomb = 0;
	Junqi *pJunqi = pEngine->pJunqi;
	ChessLineup *pLineup;
	int i;
	if( pJunqi->aInfo[iDir].bDead )
	{
		return isBomb;
	}

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->bDead || pLineup->type==NONE )
		{
			continue;
		}
		if( pLineup->type>=SILING && pLineup->type<=LVZH )
		{
			if( IsEnableMove(pJunqi, pBomb, pLineup->pChess) )
			{
				pEngine->pBomb[0] = pBomb;
				pEngine->pBomb[1] = pLineup->pChess;
				SETBIT(aEventBit, BOMB_EVENT);
				isBomb = 1;

				break;
			}
		}
	}

	return isBomb;
}

u8 ProBombEvent(Engine *pEngine)
{
	u8 isMove = 0;
	Junqi *pJunqi = pEngine->pJunqi;

	if( IsEnableMove(pJunqi, pEngine->pBomb[0], pEngine->pBomb[1]) )
	{
		isMove = 1;
		SendMove(pJunqi, pEngine->pBomb[0], pEngine->pBomb[1]);
	}
	CLEARBIT(aEventBit, BOMB_EVENT);
	return isMove;
}

void CheckBombEvent(Engine *pEngine)
{
	u8 isMove = 0;
	int i;
	Junqi *pJunqi = pEngine->pJunqi;
	ChessLineup *pLineup;
	if( ENGINE_DIR%2!=pJunqi->eTurn%2 )
	{
		return;
	}
	CLEARBIT(aEventBit, BOMB_EVENT);

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[pJunqi->eTurn][i];
		if( pLineup->type==ZHADAN && !pLineup->bDead )
		{
			isMove = CanBombChess(pEngine, pLineup->pChess, (ENGINE_DIR+1)%4);
			if( isMove )
			{
				break;
			}
			else
			{
				isMove=CanBombChess(pEngine, pLineup->pChess, (ENGINE_DIR+3)%4);
				if( isMove ) break;
			}
		}
	}

}



u8 CanEatChess(Engine *pEngine, BoardChess *pSrc, int iDir)
{
	u8 isEat = 0;
	Junqi *pJunqi = pEngine->pJunqi;
	ChessLineup *pLineup;
	int i;
	if( pJunqi->aInfo[iDir].bDead )
	{
		return isEat;
	}

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->bDead || pLineup->type==NONE )
		{
			continue;
		}

		if( IsEnableMove(pJunqi, pSrc, pLineup->pChess) )
		{
			if( pSrc->pLineup->type!=GONGB )
			{
				if( pLineup->type==DARK )
				{
					pEngine->pEat[0] = pSrc;
					pEngine->pEat[1] = pLineup->pChess;
					SETBIT(aEventBit, DARK_EVENT);
					isEat = 2;
				}
				else if( pSrc->pLineup->type<=pLineup->mx_type )
				{
					pEngine->pEat[0] = pSrc;
					pEngine->pEat[1] = pLineup->pChess;
					SETBIT(aEventBit, EAT_EVENT);
					isEat = 1;
					break;
				}
			}
			else if( pLineup->index>=20 && !pLineup->isNotLand)
			{
				pEngine->pEat[0] = pSrc;
				pEngine->pEat[1] = pLineup->pChess;
				SETBIT(aEventBit, GONGB_EVENT);
				isEat = 3;
			}

		}

	}

	return isEat;
}

void CheckEatEvent(Engine *pEngine)
{
	u8 isEat = 0;
	int i;
	Junqi *pJunqi = pEngine->pJunqi;
	ChessLineup *pLineup;
	if( ENGINE_DIR%2!=pJunqi->eTurn%2 )
	{
		return;
	}

	CLEARBIT(aEventBit, EAT_EVENT);
	CLEARBIT(aEventBit, DARK_EVENT);
	CLEARBIT(aEventBit, GONGB_EVENT);

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[pJunqi->eTurn][i];
		if( pLineup->type>=SILING && pLineup->type<=GONGB && !pLineup->bDead )
		{
			isEat = CanEatChess(pEngine, pLineup->pChess, (ENGINE_DIR+1)%4);
			if( isEat==1 )
			{
				break;
			}
			else
			{
				isEat=CanEatChess(pEngine, pLineup->pChess, (ENGINE_DIR+3)%4);
				if( isEat==1 ) break;
			}
		}
	}

}

u8 ProEatEvent(Engine *pEngine)
{
	Junqi *pJunqi = pEngine->pJunqi;

	assert( IsEnableMove(pJunqi, pEngine->pEat[0], pEngine->pEat[1]) );
	SendMove(pJunqi, pEngine->pEat[0], pEngine->pEat[1]);
	CLEARBIT(aEventBit, EAT_EVENT);
	CLEARBIT(aEventBit, DARK_EVENT);
	CLEARBIT(aEventBit, GONGB_EVENT);
	return 1;
}
