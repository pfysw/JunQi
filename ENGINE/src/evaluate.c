/*
 * evaluate.c
 *
 *  Created on: Sep 13, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "engine.h"
#include "path.h"


#undef log_a
#define log_a(format,...)

void InitValuePara(Value_Parameter *p)
{
	p->vChess[SILING] = 210;
	p->vChess[JUNZH] = 190;
	p->vChess[SHIZH] = 150;
	p->vChess[LVZH] = 100;
	p->vChess[TUANZH] = 85;
	p->vChess[YINGZH] = 30;
	p->vChess[LIANZH] = 20;
	p->vChess[PAIZH] = 15;
	p->vChess[GONGB] = 90;
	p->vChess[DILEI] = 100;
	p->vChess[ZHADAN] = 110;
	p->vDarkLand =  20;
	p->vDarkBomb =  10;
	p->vDarkJunqi = 20;
	p->vPathLand = 100;
	p->vPathChess = 50;
	p->vDanger = 100;

	p->vAllChess = p->vChess[SILING]+
	        p->vChess[JUNZH]+
	        p->vChess[SHIZH]*2+
	        p->vChess[LVZH]*2+
	        p->vChess[TUANZH]*2+
	        p->vChess[YINGZH]*2+
	        p->vChess[LIANZH]*3+
	        p->vChess[PAIZH]*3+
	        p->vChess[GONGB]*3+
	        p->vChess[DILEI]*3+
	        p->vChess[ZHADAN]*2+
	        p->vDarkLand*8+
	        p->vDarkBomb*20+
	        p->vDarkJunqi;
	log_a("all value %d",p->vAllChess);
}

int CheckCampValue(Junqi *pJunqi, int iDir)
{
    int val = 0;
    int i;
    u8 aCamp[5] = {6,8,12,16,18};
    u8 aCampVal[5] = {40,40,50,70,70};
    BoardChess *pChess;
    for(i=0; i<5; i++)
    {
        pChess = &pJunqi->ChessPos[iDir][aCamp[i]];
        if(pChess->type==NONE)
        {
            continue;
        }
        else
        {
            if( (pChess->pLineup->iDir&1)!=(iDir&1) )
            {
                val += aCampVal[i];

                if( i>2 && pJunqi->ChessPos[iDir][aCamp[i]+10].type==JUNQI)
                {
                    val += 40;
                }
                else if( i==2 && pJunqi->aInfo[iDir].bShowFlag )
                {
                    val += 40;
                }
                if( (iDir&1)!=(ENGINE_DIR&1) )
                {
                    if( i<2 )
                    {
                        //大字应该控盘而不是进营
                        if( pChess->pLineup->type<LVZH )
                        {
                            val -= 20;
                        }
                    }
                    else
                    {
                        if(pChess->pLineup->type<LVZH)
                        {
                            val -= 50;
                        }
                    }
                }
            }
            else
            {
                if( pChess->pLineup->isNotBomb )
                {
                    val += 5;
                }
            }
        }
    }
    if( pJunqi->iRpOfst>100 && pJunqi->nNoEat>10 )
    {
        log_a("no eat ignore camp ");
        val = 0;
    }
    log_a("camp %d %d",val,iDir);
    return val;
}

int CalDirPathValue(Junqi *pJunqi, int iDir, u8 *pDir)
{
    int i;
    int value = 0;
    BoardChess *pChess;
    Engine *pEngine;
    Value_Parameter *pVal;

    pEngine = pJunqi->pEngine;
    pVal= &pEngine->valPara;

    for(i=0;i<4;i++)
    {
        pChess = &pJunqi->ChessPos[iDir][pDir[i]];
        if( pChess->type!=NONE )
        {
            if((pChess->pLineup->iDir&1)==(iDir&1))
            {
                value += pVal->vPathChess;
            }
            else
            {
                value -= pVal->vPathChess;
            }
        }
    }

    return value;
}

int CalTripleValue(Junqi *pJunqi, int iDir, u8 *pDir, u8 bShowFlag)
{
    int i;
    int value = 0;
    BoardChess *pChess;
    Engine *pEngine;
    Value_Parameter *pVal;

    pEngine = pJunqi->pEngine;
    pVal= &pEngine->valPara;

    for(i=0;i<2;i++)
    {
        pChess = &pJunqi->ChessPos[iDir][pDir[i]];
        if( pChess->type!=NONE )
        {
            if((pChess->pLineup->iDir&1)==(iDir&1))
            {
                if( pChess->pLineup->isNotLand )
                {
                    value += pVal->vChess[TUANZH];
                }
                else
                {
                    value += pVal->vChess[SHIZH];
                }
            }
            else
            {
                value -= pVal->vChess[SILING];
            }
        }

        if(bShowFlag) break;
    }

    return value;
}

int CalCornerValue(Junqi *pJunqi, int iDir, u8 *pDir)
{
    int value = 0;
    BoardChess *pChess;
    Engine *pEngine;
    Value_Parameter *pVal;

    pEngine = pJunqi->pEngine;
    pVal= &pEngine->valPara;

    pChess = &pJunqi->ChessPos[iDir][pDir[0]];
    if( pChess->type!=NONE )
    {
        if((pChess->pLineup->iDir&1)==(iDir&1))
        {
            if( pChess->pLineup->isNotLand )
            {
                value += pVal->vPathChess;
            }
            else
            {
                value += pVal->vChess[LVZH];
            }
        }
        else
        {
            value -= pVal->vChess[SHIZH];
        }
    }

    return value;
}

int CalMiddleValue(Junqi *pJunqi, int iDir, u8 *pDir)
{
    int value = 0;

    value += CalCornerValue(pJunqi,iDir,pDir)>>1;
    value += CalTripleValue(pJunqi,iDir,&pDir[1],1)>>1;

    return value;
}

int CalDangerValue(Junqi *pJunqi, int iDir, u8 *pDir)
{
    int value = 0;
    BoardChess *pChess;

    int i;
    int landFlag1 = 0;
    int landFlag2 = 0;
    int landFlag3 = 0;
    Engine *pEngine;
    Value_Parameter *pVal;

    pEngine = pJunqi->pEngine;
    pVal= &pEngine->valPara;

    for(i=0;i<7;i++)
    {
        pChess = &pJunqi->ChessPos[iDir][pDir[i]];
        switch( i )
        {
        case 0:
            if( NONE==pChess->type || pChess->pLineup->isNotLand )
            {
                landFlag1 = 1;
                landFlag2 = 1;
            }
            break;
        case 1:
            if(  NONE==pChess->type || pChess->pLineup->isNotLand )
            {
                landFlag1 = 1;
            }
            break;
        case 2:
            if(  NONE==pChess->type || pChess->pLineup->isNotLand )
            {
                landFlag2 = 1;
            }
            break;
        case 3:
            if( pJunqi->aInfo[iDir].bShowFlag )
            {
                if(  NONE==pChess->type || pChess->pLineup->isNotLand )
                {
                    landFlag3 = 1;
                }
            }
            break;
        default:
            break;
        }

        if( pChess->type!=NONE )
        {
            if((pChess->pLineup->iDir&1)!=(iDir&1))
            {
                if( iDir==ENGINE_DIR || pChess->pLineup->type<YINGZH )
                {
                    if(i<3)
                    {
                        value += pVal->vDanger<<2;
                    }
                    else
                    {

                        //value += 100;
                        if( 3==i && landFlag1 )
                        {
                            value += pVal->vDanger<<1;
                        }
                        else if( landFlag2 )
                        {
                            value += pVal->vDanger;
                        }
                    }
                }
            }

        }
    }
    if( pJunqi->iRpOfst<100 || (iDir&1)!=ENGINE_DIR )//todo
    {

        if( landFlag1 )
        {
            log_a("iRpOfst %d %d",pJunqi->iRpOfst,iDir);
            value += pVal->vDanger<<1;
        }
        else if( landFlag2 )
        {
            value += pVal->vDanger;
        }
        else if( landFlag3 )
        {
            value += pVal->vDanger>>1;
        }
    }


    return value;
}

#if 0
int CheckPathValue(Junqi *pJunqi, int iDir)
{
    int value = 0;
    int val[7] = {0};
    int vLeft = 0;
    int vRight = 0;
    int vDanger = 0;
    u8 bShowFlag;
    u8 bLeft = 0;

    //数值有先后顺序的要求
    u8 aLeft[4] = {4,9,14,19};
    u8 aRight[4] = {0,5,10,15};
    u8 aLeftTriple[3] = {23,29,27};
    u8 aRightTriple[3] = {21,25,27};
    u8 aLeftCorner[2] = {24,22};
    u8 aRightCorner[2] = {20,22};
    u8 aMiddle[2] = {22,27};
    u8 aLeftBarrier[5] = {23,29,27,24,22};
    u8 aRightBarrier[5] = {21,25,27,20,22};

    bShowFlag = pJunqi->aInfo[iDir].bShowFlag;


    val[0] = CalDirPathValue(pJunqi,iDir,aLeft);
    val[1] = CalDirPathValue(pJunqi,iDir,aRight);
    if( bShowFlag )
    {
        if( pJunqi->Lineup[iDir][28].type==JUNQI )
        {
            bLeft = 1;
        }
        else
        {
            bLeft = 0;
        }
        val[2] = CalTripleValue(pJunqi,iDir,aLeftTriple,!bLeft);
        val[3] = CalTripleValue(pJunqi,iDir,aRightTriple,bLeft);
    }
    else
    {
        val[2] = CalTripleValue(pJunqi,iDir,aLeftTriple,0);
        val[3] = CalTripleValue(pJunqi,iDir,aRightTriple,0);
    }

    val[4] = CalCornerValue(pJunqi,iDir,aLeftCorner);
    val[5] = CalCornerValue(pJunqi,iDir,aRightCorner);
    val[6] = CalMiddleValue(pJunqi,iDir,aMiddle);

//    for(int i=0;i<7;i++)
//    {
//        log_a("val %d %d",i,val[i]);
//    }
    vLeft = val[0]+val[2]+val[4];
    vRight = val[1]+val[3]+val[5];
    if( bShowFlag )
    {
        if( bLeft )
        {
            vDanger = CalDangerValue(pJunqi,iDir,aLeftBarrier);
            value = vLeft+(vRight>>1)+val[6]+val[0]+vDanger;
        }
        else
        {
            assert( pJunqi->Lineup[iDir][26].type==JUNQI );
            vDanger = CalDangerValue(pJunqi,iDir,aRightBarrier);
            value = vRight+(vLeft>>1)+val[6]+val[1]+vDanger;
            //log_a("danger %d vRight %d vLeft %d",vDanger,vRight,vLeft>>1);
        }
    }
    else
    {
        value = vLeft+vRight+val[6];
    }
    //log_a("dir %d %d",iDir,value);
    return value;
}
#endif

int CheckDangerValue(Junqi *pJunqi, int iDir)
{
    int vDanger = 0;
    u8 bShowFlag;

    u8 aLeftBarrier[7] = {23,29,27,24,22,20,21};
    u8 aRightBarrier[7] = {21,25,27,20,22,23,24};
    u8 aBarrier[7] = {21,25,23,20,24,22,29};

    bShowFlag = pJunqi->aInfo[iDir].bShowFlag;

    if( bShowFlag )
    {
        if( pJunqi->Lineup[iDir][28].type==JUNQI )
        {
            vDanger = CalDangerValue(pJunqi,iDir,aLeftBarrier);
        }
        else
        {
            vDanger = CalDangerValue(pJunqi,iDir,aRightBarrier);
        }
    }
    else if( (iDir&1)!=(ENGINE_DIR&1) )
    {
        vDanger = CalDangerValue(pJunqi,iDir,aBarrier);
    }
    log_a("dir %d vDanger %d",iDir,vDanger);
    return vDanger;
}

int CheckMaxChess(Junqi *pJunqi, int aMaxNum[])
{
    int nMax = 0;
    int rc = 0;
    int num;
    u8 i;
    u8 type;


    for(type=SILING; type<TUANZH; type++)
    {
        nMax = 0;
        for(i=0; i<4; i++)
        {
            if( pJunqi->aInfo[i].bDead ) continue;


            if( (i&1)==(ENGINE_DIR&1) )
            {
                num = pJunqi->aInfo[i].aTypeNum[type];
                nMax += num;
            }
            else
            {
                num = pJunqi->aInfo[i].aLiveAllNum[type] -
                        pJunqi->aInfo[i].aLiveAllNum[type-1];
                nMax -= num;
            }
        }
        aMaxNum[type] = nMax;
        if( !rc && nMax )
        {
            rc = nMax;
        }
    }

    return rc;
}

int CalMaxChessValue(Junqi *pJunqi, int aMaxNum[])
{
    int value = 0;
    Engine *pEngine;
    Value_Parameter *pVal;
    pEngine = pJunqi->pEngine;
    pVal= &pEngine->valPara;
    int type;

    for(type=SILING; type<TUANZH; type++)
    {
        value += pVal->vChess[type]*aMaxNum[type];
    }

    return value;
}

int EvalSituation(Junqi *pJunqi)
{
	int i,j;
	ChessLineup *pLineup;
	int tempValue = 0;
	int value = 0;;
	Engine *pEngine;
	Value_Parameter *pVal;
	int aMaxNum[14] = {0};
	int vDeltaBomb = 0;
	int rc = 0;
	int vMaxChess = 0;

	pEngine = pJunqi->pEngine;
	pVal= &pEngine->valPara;



	rc = CheckMaxChess(pJunqi,aMaxNum);
	if( rc>0 )
	{
	    vDeltaBomb = 30;
	}
	else
	{
	    vDeltaBomb = -30;
	}
	if( pJunqi->iRpOfst<100 )
	{
	    vMaxChess = CalMaxChessValue(pJunqi,aMaxNum);
	}
	log_a("max %d rc %d",vMaxChess,rc);
    value += vMaxChess;
	for(i=0; i<4; i++)
	{
	    tempValue = 0;

		if( !pJunqi->aInfo[i].bDead || !pJunqi->aInfo[i].deadValue )
		{
		    //assert(i==1);

//			log_b("i %d dilei %d zhadan %d",i,
//					pJunqi->aInfo[i].aTypeNum[DILEI],
//					pJunqi->aInfo[i].aTypeNum[ZHADAN]);
			for(j=0; j<30; j++)
			{
				pLineup = &pJunqi->Lineup[i][j];
				if( pLineup->type!=NONE && pLineup->type!=JUNQI )
				{
					if( i%2==ENGINE_DIR%2 )
					{

						if( pLineup->bDead ||
						    (pLineup->pChess->isStronghold && pLineup->pChess->iDir!=i) )
						{

							tempValue -= pVal->vChess[pLineup->type];
							if( pLineup->index>=20 )
							{
								tempValue -= (pVal->vDarkBomb+pVal->vDarkLand);
							}
							else if(  pLineup->index>=5 )
							{
								tempValue -= pVal->vDarkBomb;
							}
							if( pLineup->type==ZHADAN )
							{
							    tempValue += vDeltaBomb;//令子多则减少炸弹价值，否则增加炸弹价值
							}
						}
						else
						{
							if( pLineup->isNotLand && pLineup->index>=20 )
							{
								tempValue -= pVal->vDarkLand;
							}

							if( pLineup->isNotBomb && pLineup->index>=5 )
							{
								tempValue -= pVal->vDarkBomb;
							}
//							if( pLineup->nEat>1 )
//							{
//							    value += (pLineup->nEat-1)*20;
//							}
						}

					}
					else
					{
						if( pLineup->bDead ||
						    (pLineup->pChess->isStronghold && pLineup->pChess->iDir!=i) )
						{
							if( pLineup->type==DARK )
							{
								tempValue += pVal->vChess[pLineup->mx_type]/2;

							}
							else
							{
								if( pLineup->bBomb )
								{
								    tempValue += (pVal->vChess[pLineup->type]+
								    	 	pVal->vChess[ZHADAN]+vDeltaBomb)/2;
								}
								else if( pLineup->type==GONGB || pLineup->type==DILEI )
								{
									tempValue += pVal->vChess[pLineup->type];
								}
								else
								{
								    tempValue += (pVal->vChess[pLineup->type]+
								    	 	pVal->vChess[pLineup->mx_type])/2;
								}
							}
							//死掉的棋需要加回暗信息分数
							if( pLineup->index>=20 )
							{
								tempValue += (pVal->vDarkBomb+pVal->vDarkLand);
							}
							else if(  pLineup->index>=5 )
							{
								tempValue += pVal->vDarkBomb;
							}

                            if( pLineup->type==ZHADAN )
                            {
                                tempValue += vDeltaBomb;//令子多则减少炸弹价值，否则增加炸弹价值
                            }
						}
						else
						{
							if( pLineup->isNotLand && pLineup->index>=20 )
							{
								tempValue += pVal->vDarkLand;
							}
							if( ( pLineup->isNotBomb || 2==pJunqi->aInfo[i].aTypeNum[ZHADAN] )
									&& pLineup->index>=5 )
							{
								tempValue += pVal->vDarkBomb;
							}
//                            if( pLineup->nEat>1 )
//                            {
//                                value -= (pLineup->nEat-1)*20;
//                            }
						}
					}

				}
			}

            if( i%2==ENGINE_DIR%2 )
            {
                log_a("temp value %d %d",i,tempValue);
                //tempValue -= (CheckDangerValue(pJunqi,i)>>1);
                tempValue -= CheckDangerValue(pJunqi,i);
                tempValue -= CheckCampValue(pJunqi,i);
                if( pJunqi->iRpOfst<100 || (i&1)!=ENGINE_DIR )//todo
                {
                    if( pJunqi->aInfo[i].bShowFlag )
                    {
                        tempValue -= (pVal->vDarkBomb+pVal->vDarkLand);
                        tempValue -= pVal->vDarkJunqi;
                    }
                }
                log_a("temp value %d %d",i,tempValue);
                if( pJunqi->aInfo[i].bDead )
                {
                    if( tempValue>-pVal->vAllChess )
                    {
                        tempValue = -(pVal->vAllChess+(pVal->vDanger<<2)+50);
                    }
                    if( pJunqi->aInfo[(i+2)&3].bDead)
                    {
                        tempValue -= 1000;
                    }
                    pJunqi->aInfo[i].deadValue = tempValue;

                }
            }
            else
            {
                tempValue += CheckDangerValue(pJunqi,i);
                tempValue += CheckCampValue(pJunqi,i);
                if( pJunqi->aInfo[i].bShowFlag )
                {
                    tempValue += (pVal->vDarkBomb+pVal->vDarkLand);
                    tempValue += pVal->vDarkJunqi;
                }
                log_a("temp value %d %d",i,tempValue);
                if( pJunqi->aInfo[i].bDead )
                {
                    if( tempValue<pVal->vAllChess )
                    {
                        tempValue = pVal->vAllChess+(pVal->vDanger<<2)+50;
                    }
                    if( pJunqi->aInfo[(i+2)&3].bDead)
                    {
                        tempValue = tempValue+1000;
                    }
                    pJunqi->aInfo[i].deadValue = tempValue;
                }
            }

		}
		else
		{
			if( i%2==ENGINE_DIR%2 )
			{
				tempValue = pJunqi->aInfo[i].deadValue;
			}
			else
			{
				tempValue = pJunqi->aInfo[i].deadValue;
			}
			log_a("dead value %d %d",i,tempValue);
		}
		value += tempValue;
	}

	return value;
}
