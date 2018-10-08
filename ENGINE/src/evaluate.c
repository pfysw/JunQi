/*
 * evaluate.c
 *
 *  Created on: Sep 13, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "engine.h"

void InitValuePara(Value_Parameter *p)
{
	p->vAllChess = 1600;
	p->vChess[SILING] = 100;
	p->vChess[JUNZH] = 90;
	p->vChess[SHIZH] = 80;
	p->vChess[LVZH] = 70;
	p->vChess[TUANZH] = 60;
	p->vChess[YINGZH] = 50;
	p->vChess[LIANZH] = 40;
	p->vChess[PAIZH] = 30;
	p->vChess[GONGB] = 55;
	p->vChess[DILEI] = 60;
	p->vChess[ZHADAN] = 65;
	p->vDarkLand =  10;
	p->vDarkBomb =  4;
	p->vDarkJunqi = 10;
}

int EvalSituation(Junqi *pJunqi)
{
	int i,j;
	ChessLineup *pLineup;
	int value = 0;
	Engine *pEngine;
	Value_Parameter *pVal;

	pEngine = pJunqi->pEngine;
	pVal= &pEngine->valPara;
	for(i=0; i<4; i++)
	{
		if( !pJunqi->aInfo[i].bDead)
		{
			if( pJunqi->aInfo[i].bShowFlag )
			{
				if( i%2==ENGINE_DIR%2 )
				{
					value -= pVal->vDarkJunqi;
				}
				else
				{
					value += pVal->vDarkJunqi;
				}
			}
//			log_b("i %d dilei %d zhadan %d",i,
//					pJunqi->aInfo[i].aTypeNum[DILEI],
//					pJunqi->aInfo[i].aTypeNum[ZHADAN]);
			for(j=0; j<30; j++)
			{
				pLineup = &pJunqi->Lineup[i][j];
				if( pLineup->type!=NONE )
				{
					if( i%2==ENGINE_DIR%2 )
					{
						if( pLineup->bDead )
						{
							value -= pVal->vChess[pLineup->type];
							if( pLineup->index>=20 )
							{
								value -= (pVal->vDarkBomb+pVal->vDarkLand);
							}
							else if(  pLineup->index>=5 )
							{
								value -= pVal->vDarkBomb;
							}
						}
						else
						{
							if( pLineup->isNotLand && pLineup->index>=20 )
							{
								value -= pVal->vDarkLand;
							}
							if( ( pLineup->isNotBomb || 2==pJunqi->aInfo[i].aTypeNum[ZHADAN] )
									&& pLineup->index>=5 )
							{
								value -= pVal->vDarkBomb;
							}
						}
					}
					else
					{
						if( pLineup->bDead )
						{
							if( pLineup->type==DARK )
							{
								value += pVal->vChess[pLineup->mx_type]/2;

							}
							else
							{
								if( pLineup->bBomb )
								{
								    value += (pVal->vChess[pLineup->type]+
								    	 	pVal->vChess[ZHADAN])/2;
								}
								else if( pLineup->type==GONGB || pLineup->type==DILEI )
								{
									value += pVal->vChess[pLineup->type];
								}
								else
								{
								    value += (pVal->vChess[pLineup->type]+
								    	 	pVal->vChess[pLineup->mx_type])/2;
								}
							}
							//死掉的棋需要加回暗信息分数
							if( pLineup->index>=20 )
							{
								value += (pVal->vDarkBomb+pVal->vDarkLand);
							}
							else if(  pLineup->index>=5 )
							{
								value += pVal->vDarkBomb;
							}
						}
						else
						{
							if( pLineup->isNotLand && pLineup->index>=20 )
							{
								value += pVal->vDarkLand;
							}
							if( ( pLineup->isNotBomb || 2==pJunqi->aInfo[i].aTypeNum[ZHADAN] )
									&& pLineup->index>=5 )
							{
								value += pVal->vDarkBomb;
							}
						}

					}

				}
			}
		}
		else
		{
			if( i%2==ENGINE_DIR%2 )
			{
				value -= pVal->vAllChess;
			}
			else
			{
				value += pVal->vAllChess;
			}
		}
	}

	return value;
}
