/*
 * movegen.c
 *
 *  Created on: Sep 21, 2018
 *      Author: Administrator
 */
#include "engine.h"
#include "junqi.h"
#include "movegen.h"
#include "path.h"

void InsertMoveList(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pData
	)
{
	MoveList *pMove;
	MoveList *pHead = pJunqi->pMoveList;

	pMove = (MoveList *)malloc(sizeof(MoveList));
	memset(pMove, 0, sizeof(MoveList));
	pMove->move.src[0] = pSrc->point.x;
	pMove->move.src[1] = pSrc->point.y;
	pMove->move.dst[0] = pDst->point.x;
	pMove->move.dst[1] = pDst->point.y;
	memcpy(&pMove->move.result, &pData->result,  sizeof(MoveResultData)-4);
    SafeMemout((u8*)&pMove->move,10);
	if( pHead==NULL )
	{
		pMove->isHead = 1;
		pMove->pNext = pMove;
		pMove->pPre = pMove;
		pJunqi->pMoveList = pMove;
	}
	else
	{
		pMove->pNext = pHead;
		pMove->pPre = pHead->pPre;
		pHead->pPre->pNext = pMove;
		pHead->pPre = pMove;
	}
}

void AddJunqiMove(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp
	)
{


	if( !pJunqi->aInfo[pDst->pLineup->iDir].bShowFlag )
	{
		pTemp->extra_info |= 1;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
		pTemp->extra_info &= ~1;//第1个bit清0
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
	}
	else if( pDst->pLineup->type==JUNQI )
	{
		pTemp->extra_info |= 1;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
	}
	else
	{
		pTemp->extra_info &= ~1;//第1个bit清0
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
	}
}

BoardChess *ShowBanner(Junqi *pJunqi, int iDir)
{
	BoardChess *pBanner;
	if( pJunqi->ChessPos[iDir][26].type==JUNQI )
	{
		pBanner = &pJunqi->ChessPos[iDir][26];
	}
	else
	{
		pBanner = &pJunqi->ChessPos[iDir][28];
	}
	return pBanner;
}

void AddJunqiPos(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp,
	u8 extra_info
	)
{
	BoardChess *pChess;
	BoardChess *pBanner;

	pTemp->extra_info |= extra_info;
	assert( extra_info==2 || extra_info==4 );
	if(extra_info==2)
	{
		pChess = pSrc;
	}
	else
	{
		pChess = pDst;
	}
	if( pJunqi->aInfo[pChess->pLineup->iDir].bShowFlag )
	{
		pBanner = ShowBanner(pJunqi,pChess->pLineup->iDir);
		pTemp->junqi_dst[0] = pBanner->point.x;
		pTemp->junqi_dst[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
	}
	else
	{
		pBanner = &pJunqi->ChessPos[pChess->pLineup->iDir][26];
		pTemp->junqi_dst[0] = pBanner->point.x;
		pTemp->junqi_dst[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
		pBanner = &pJunqi->ChessPos[pChess->pLineup->iDir][28];
		pTemp->junqi_dst[0] = pBanner->point.x;
		pTemp->junqi_dst[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp);
	}
}


void AddCommanderMove(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp
	)
{

	MoveList *pHead = pJunqi->pMoveList;

    //不考虑自家司令阵亡记录的信息，因为这是本来就知道的事情
	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if( pSrc->pLineup->type==SILING || pSrc->pLineup->type==ZHADAN )
		{
			//司令没死
			if( (pJunqi->aInfo[pDst->pLineup->iDir].bShowFlag&2)==0 )
			{
				AddJunqiPos(pJunqi,pSrc,pDst,pTemp,4);
			}
		}
	}
	else
	{
		if( pDst->pLineup->type==SILING || pDst->pLineup->type==ZHADAN )
		{
			//司令没死
			if( (pJunqi->aInfo[pSrc->pLineup->iDir].bShowFlag&2)==0 )
			{
				AddJunqiPos(pJunqi,pSrc,pDst,pTemp,2);
			}
		}
	}
}

void AddCommanderKilled(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp
	)
{

	if( pSrc->pLineup->iDir%2!=ENGINE_DIR%2 )
	{
		if( (pJunqi->aInfo[pSrc->pLineup->iDir].bShowFlag&2)==0 &&
			pDst->pLineup->type==DILEI )
		{
			AddJunqiPos(pJunqi,pSrc,pDst,pTemp,2);
		}

	}
}


u8 IsPossibleEat(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst)
{
	u8 rc = 1;

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if(pSrc->pLineup->type>=pDst->pLineup->type &&
				pDst->pLineup->type>=SILING )
		{
			return  0;
		}
		if( pSrc->pLineup->type==GONGB  &&
			(pDst->pLineup->isNotLand || pDst->pLineup->index<20) )
		{
			return  0;
		}
		if( pSrc->pLineup->type==ZHADAN ) return  0;
	}
	else
	{
		if( pSrc->pLineup->mx_type>=pDst->pLineup->type &&
				pDst->pLineup->type>=SILING )
		{
			return  0;
		}
		if( pDst->pLineup->type==DILEI &&
			pSrc->pLineup->type!=GONGB &&
			(pSrc->pLineup->type!=DARK ||
			  pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB]==3) )
		{
			return  0;
		}
		if( pDst->pLineup->type==ZHADAN ) return  0;
	}

	return rc;
}

u8 IsPossibleKilled(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst)
{
	u8 rc = 1;

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		//比敌方最大的棋还大
		if( pSrc->pLineup->type<=pDst->pLineup->mx_type &&
			pDst->pLineup->isNotLand )
		{
			return  0;
		}
	}
	else
	{
		//最小的棋都比自己大,如果Dst是地雷进不了该条件
		if( pSrc->pLineup->type<=pDst->pLineup->type &&
			pDst->pLineup->type>=SILING &&
			pSrc->pLineup->type!=DARK )
		{
			return  0;
		}
		if( pDst->pLineup->type==DILEI && pSrc->pLineup->type==GONGB )
		{
			return  0;
		}
		if( pDst->pLineup->type==ZHADAN ) return  0;
	}

	return rc;
}

u8 IsPossibleBomb(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst)
{
	u8 rc = 1;

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		//敌方明棋
		if( pDst->pLineup->type>=SILING )
		{
			//双方都不可能是炸，不可能打兑
			if( pSrc->pLineup->type>pDst->pLineup->type )
			{
				return  0;
			}
			if( pSrc->pLineup->type!=ZHADAN )
			{
				if( pSrc->pLineup->type<pDst->pLineup->mx_type )
					return 0;
			}
		}
		//只有炸弹才能和地雷军旗打兑
		if( pSrc->pLineup->type!=ZHADAN &&
			(pDst->pLineup->type==DILEI || pDst->pLineup->type==JUNQI) )
		{
			return  0;
		}
	}
	else
	{
		if( pDst->pLineup->type>=SILING )
		{
			//如果对方是暗棋，怎么都有可能打兑
			if( pSrc->pLineup->type!=DARK )
			{
				//双方都不可能是炸弹
				if( pSrc->pLineup->mx_type>pDst->pLineup->type )
				{
					return  0;
				}
				//Dst小于Src最小可能的类型
				if( pSrc->pLineup->type<pDst->pLineup->type )
					return 0;
			}
		}
		//只有炸弹才能和地雷军旗打兑
		if( pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN]==2 &&
			(pDst->pLineup->type==DILEI || pDst->pLineup->type==JUNQI) )
		{
			return  0;
		}
	}

	return rc;
}

int SpecialCase(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst)
{
	int rc = 0;
	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if( pSrc->pLineup->type==SILING )
		{
			if( pDst->pLineup->isNotBomb )
			{
				return 1;
			}
		}
	}
	else
	{
		if( pDst->pLineup->type==SILING )
		{
			if( pSrc->pLineup->isNotBomb )
			{
				return 1;
			}
		}
	}

	return rc;
}

void AddMoveToList(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst)
{

	MoveResultData temp;
	enum CompareType type;

	for(type=MOVE; type<=KILLED; type++)
	{
		memset(&temp, 0 ,sizeof(temp));
		temp.result = type;
		if( pDst->type!=NONE && type == MOVE )
		{
			continue;
		}
		switch(type)
		{
		case MOVE:
			InsertMoveList(pJunqi,pSrc,pDst,&temp);
			break;
		case EAT:
            if( !IsPossibleEat(pJunqi,pSrc,pDst) )
            {
            	continue;
            }
        	if( pDst->isStronghold )
        	{
        		AddJunqiMove(pJunqi,pSrc,pDst,&temp);
        	}
        	else
        	{
        		InsertMoveList(pJunqi,pSrc,pDst,&temp);
        	}
			break;
		case BOMB:
            if( !IsPossibleBomb(pJunqi,pSrc,pDst) )
            {
            	continue;
            }
        	if( pDst->isStronghold )
        	{
        		AddJunqiMove(pJunqi,pSrc,pDst,&temp);
        	}
        	//暂时不考虑大本营是司令的情况
        	else
        	{
        		//己方司令不可能碰到第一排被炸
        		if( !SpecialCase(pJunqi,pSrc,pDst) )
        		{
        			InsertMoveList(pJunqi,pSrc,pDst,&temp);
        		}
        		AddCommanderMove(pJunqi,pSrc,pDst,&temp);
        	}
			break;
		case KILLED:

            if( !IsPossibleKilled(pJunqi,pSrc,pDst) )
            {
            	continue;
            }
            InsertMoveList(pJunqi,pSrc,pDst,&temp);
            AddCommanderKilled(pJunqi,pSrc,pDst,&temp);
			break;
		default:
			break;
		}

		 //只能是MOVE
		if( pDst->type==NONE )
		{
			break;
		}
	}
}

BoardChess * GetValideMove(Junqi* pJunqi, BoardChess *pSrc, int j)
{
	BoardChess *pDst=NULL;
	BoardChess *pTemp;

	if( j<120 )
	{
		pTemp = &pJunqi->ChessPos[j/30][j%30];
	}
	else
	{
		pTemp = &pJunqi->NineGrid[j-120];
	}
	if( pTemp->type!=NONE && pSrc->pLineup->iDir%2==pTemp->pLineup->iDir%2 )
	{
		return pDst;
	}
	if( IsEnableMove(pJunqi, pSrc, pTemp) )
	{
		pDst = pTemp;
	}

	return pDst;
}

void ClearMoveList(Junqi *pJunqi)
{
	MoveList *p;
	MoveList *pTmp;
	MoveList *pHead = pJunqi->pMoveList;

	if( pHead==NULL )
	{
		return;
	}
	p = pHead->pNext;

	while(1)
	{
		if(p->isHead)
		{
			free(p);
			pJunqi->pMoveList = NULL;
			break;
		}
		else
		{
			pTmp = p;
			p = p->pNext;
			free(pTmp);
		}

	}

}
void GenerateMoveList(Junqi* pJunqi)
{
    int i,j;
    BoardChess *pSrc;
    BoardChess *pDst;
    ChessLineup *pLineup;
    int temp[3] = {3,7,5};

    for(i=0;  i<30; i++)
    {
    	pLineup = &pJunqi->Lineup[pJunqi->eTurn][i];
    	if( pLineup->bDead )
    	{
    		continue;
    	}
    	pSrc = pLineup->pChess;

    	if(pLineup->type!=NONE && pLineup->type!=JUNQI && pLineup->type!=DILEI )
    	{
    		for(j=0; j<129; j++)
    		{
    			temp[0] = pSrc->type;
    			//只有敌方的棋才可能是暗棋,只考虑没碰撞过的
    			//pSrc->pLineup->type是dark，那么pSrc->type必然是dark，
    			//反之则不一定，反之则不一定，pSrc->type只是确定有无棋子，而不管棋子是什么
        		if( pSrc->pLineup->type==DARK && pSrc->isRailway &&
        			pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB]<3 )
        		{
        			 //暂时设置棋子的位置为工兵，获取工兵路线
        			//棋子的类型pSrc->pLineup->type还是dark没有变
        			assert( pSrc->type==DARK );
        			pSrc->type = GONGB;
        		}
    			pDst = GetValideMove(pJunqi, pSrc, j);
    			pSrc->type = temp[0];
    			temp[1] = pSrc->pLineup->mx_type;
    			temp[2] = pSrc->pLineup->type;


        		if( pDst!=NULL )
        		{
        	   		if( pSrc->pLineup->type==DARK )
    				{
        	   			//如果得到的是工兵路线，则按工兵处理
        	   			if( !IsEnableMove(pJunqi, pSrc, pDst) )
        	   			{
        	   				pSrc->pLineup->type = GONGB;
        	   				pSrc->pLineup->mx_type = GONGB;
        	   			}
    				}
        			AddMoveToList(pJunqi, pSrc, pDst);
        			log_a("i %d j %d src %d %d dst %d %d",i,j,
        					pSrc->point.x,pSrc->point.y,
        					pDst->point.x,pDst->point.y);

        		}
        		//恢复类型
        		pSrc->pLineup->mx_type = temp[1];
        		pSrc->pLineup->type = temp[2];
    		}

    	}
    }

}
