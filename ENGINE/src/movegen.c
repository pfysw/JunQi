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
#include "search.h"



void InsertMoveList(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pData,
	int percent
	)
{
	MoveList *pMove;
	MoveList *pHead = pJunqi->pMoveList;

	pJunqi->test_gen_num++;
	//pMove = (MoveList *)malloc(sizeof(MoveList));
	//pMove = MoveNodeMalloc(pJunqi);
	pMove = (MoveList *)memsys5Malloc(pJunqi,sizeof(MoveList));

	memset(pMove, 0, sizeof(MoveList));
	pMove->move.src[0] = pSrc->point.x;
	pMove->move.src[1] = pSrc->point.y;
	pMove->move.dst[0] = pDst->point.x;
	pMove->move.dst[1] = pDst->point.y;
	memcpy(&pMove->move.result, &pData->result,  sizeof(MoveResultData)-4);
	pMove->percent = percent;
//	if(pJunqi->test_gen_num==7)
//	    log_c("test");
//	log_a("gen num %d", pJunqi->test_gen_num);
//    SafeMemout((u8*)&pMove->move,10);
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

int AddJunqiMove(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp
	)
{
	int percent = 0;
	int percent1 = 0;//吃到军旗
	int percent2 = 0;//没吃到

	log_b("stronghold");

	////////
    if( pSrc->pLineup->type==ZHADAN )
    {
        if( pDst->pLineup->type!=JUNQI )
        {
            assert( pTemp->result==BOMB );
            percent1 = 128;
            percent2 = 128;
        }
        else
        {
            pTemp->extra_info |= 1;
            percent = 256;
            InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent);
        }
    }
    else
    {
        if( pDst->pLineup->type==JUNQI )
        {
            assert( pTemp->result==EAT );
            pTemp->extra_info |= 1;
            percent = 256;
            InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent);
        }
        else
        {
            if( pTemp->result==EAT )
            {
                percent1 = 128;
                percent2 = 80;
            }
            else
            {
                percent1 = 0;
                percent2 = 20;
            }
        }
    }

	if( !pJunqi->aInfo[pDst->pLineup->iDir].bShowFlag )
	{
	    if( pDst->pLineup->iDir%2!=ENGINE_DIR )
	        assert( pDst->pLineup->type!=JUNQI );
	    if( percent1!=0 )
	    {
            pTemp->extra_info |= 1;
            InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent1);
	    }
		pTemp->extra_info &= ~1;//第1个bit清0
		InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent2);
		percent = percent1+percent2;
	}

	return percent;
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
	u8 extra_info,
	int percent
	)
{
	BoardChess *pChess;
	BoardChess *pBanner;
	u8 *junqi_dir;

	pTemp->extra_info |= extra_info;
	assert( extra_info==2 || extra_info==4 );
	if(extra_info==2)
	{
		pChess = pSrc;
		junqi_dir = pTemp->junqi_src;
	}
	else
	{
		pChess = pDst;
		junqi_dir = pTemp->junqi_dst;
	}
	if( pJunqi->aInfo[pChess->pLineup->iDir].bShowFlag )
	{
		pBanner = ShowBanner(pJunqi,pChess->pLineup->iDir);
		junqi_dir[0] = pBanner->point.x;
		junqi_dir[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent);
	}
	else
	{
		pBanner = &pJunqi->ChessPos[pChess->pLineup->iDir][26];
		junqi_dir[0] = pBanner->point.x;
		junqi_dir[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent>>1);
		pBanner = &pJunqi->ChessPos[pChess->pLineup->iDir][28];
		junqi_dir[0] = pBanner->point.x;
		junqi_dir[1] = pBanner->point.y;
		InsertMoveList(pJunqi,pSrc,pDst,pTemp,percent>>1);
	}
}


int AddCommanderMove(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp,
	int per
	)
{

	int percent = 0;
	int nBomb = 0;

    //不考虑自家司令阵亡记录的信息，因为这是本来就知道的事情
	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if( pSrc->pLineup->type==SILING || pSrc->pLineup->type==ZHADAN )
		{
			//司令没死
			//if( (pJunqi->aInfo[pDst->pLineup->iDir].bShowFlag&2)==0 )
			if( pDst->pLineup->mx_type==SILING )//可能司令明了但还活着
			{

				if( pSrc->pLineup->type==ZHADAN )
				{
				    if(pDst->pLineup->type!=SILING)
				    {
				        percent = per>>3;// per/8
				    }
				    else
				    {
				        percent = per;
				    }
				}
				else
				{
					if( pDst->pLineup->isNotBomb )
					{
						percent = per;
					}
					else
					{
					    nBomb = 2-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[ZHADAN];
						percent = per/(1+nBomb);
					}
				}
				AddJunqiPos(pJunqi,pSrc,pDst,pTemp,4,percent);
			}
		}
	}
	else
	{
		if( pDst->pLineup->type==SILING || pDst->pLineup->type==ZHADAN )
		{
			//司令没死
			//if( (pJunqi->aInfo[pSrc->pLineup->iDir].bShowFlag&2)==0 )
		    if( pSrc->pLineup->mx_type==SILING )//可能司令明了但还活着
			{
				if( pDst->pLineup->type==ZHADAN )
				{
                    if(pSrc->pLineup->type!=SILING)
                    {
                        percent = per>>3;// per/8
                    }
                    else
                    {
                        percent = per;
                    }
				}
				else
				{
					if( pSrc->pLineup->isNotBomb )
					{
						percent = per;
					}
					else
					{
                        nBomb = 2-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN];
                        percent = per/(1+nBomb);
					}
				}
				AddJunqiPos(pJunqi,pSrc,pDst,pTemp,2,percent);
			}
		}
	}
	memset(&pTemp->extra_info,0,5);

	return percent;
}

void AddCommanderKilled(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pTemp,
	int percent
	)
{

	if( pSrc->pLineup->iDir%2!=ENGINE_DIR%2 )
	{
		if( (pJunqi->aInfo[pSrc->pLineup->iDir].bShowFlag&2)==0 &&
			pDst->pLineup->type==DILEI )
		{
			AddJunqiPos(pJunqi,pSrc,pDst,pTemp,2,percent);
		}

	}
}

int GetEatPercent(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst)
{
	u16 percent = 0;
	u8 *aLiveTypeSum;
	u8 *aLiveTypeAll;//aLiveTypeAll[x]==0  如果x<SILING
	int src = pSrc->pLineup->type;
	int dst = pDst->pLineup->type;
	int mxSrcType = pSrc->pLineup->mx_type;
	int mxDstType = pDst->pLineup->mx_type;


	int nBomb = 0;
	int nLand = 0;
	int nSapper = 0;
	int num;
	int mxNum;
	int nSrc;
	int nDst;
	int nMayLand;
	int nMayBomb;
	int nMayBombLand;
	int tempBomb,tempLand;
	int nTemp;



	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		aLiveTypeSum = pJunqi->aInfo[pDst->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pDst->pLineup->iDir].aLiveAllNum;
		nMayLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayLand;
		nMayBomb = pJunqi->aInfo[pDst->pLineup->iDir].nMayBomb;
		nMayBombLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayBombLand;

		nBomb = 2-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[ZHADAN];
		nLand = 3-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[DILEI];

		if( pSrc->pLineup->type!=GONGB)
		{
			if( pDst->pLineup->type==DARK )
			{
			    log_b("dark");
                num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
                mxNum = (aLiveTypeAll[mxDstType-1]-aLiveTypeSum[mxDstType-1]);
                nSrc = aLiveTypeAll[src]-aLiveTypeSum[src];
                log_b("%d %d %d %d",aLiveTypeAll[GONGB],aLiveTypeSum[GONGB],nBomb,nLand);
                log_b("%d %d %d",mxDstType,aLiveTypeAll[mxDstType-1],aLiveTypeSum[mxDstType-1]);
                log_b("%d %d",aLiveTypeAll[src],aLiveTypeSum[src]);
                nSrc = (src>=mxDstType)?nSrc:mxNum;
                log_b("num %d max %d",num,mxNum);
                log_b("src %s %d",aTypeName[src],nSrc);
			    if( pDst->pLineup->isNotBomb )
			    {
			        //用2^8代替百分比
                    percent = ((num-nSrc)<<8)/(num-mxNum);
			    }
			    else
			    {
                    //原始nMayBomb中可能含有是地雷，这个不再统计范围内
                    //可能是地雷和炸弹的棋，减去重复的，再减去地雷
                    nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;
                    log_b("nMayBomb %d %d",nMayBomb,nBomb);
			        if( pDst->pLineup->isNotLand || 0==nMayLand )
			        {

			            if( nMayBomb==0 )
			            {

			                percent = ((num-nSrc)<<8)/(num-mxNum);
			            }
			            else
			            {

			                tempBomb = nMayBomb-nBomb;
			                if( tempBomb<0 ) tempBomb= 0;
                            //percent = p(吃掉)*(1-(p(炸弹))
                            //p(炸弹) = 1-nBomb/nMayBomb
                            percent = ((num-nSrc)<<8)*(tempBomb)/((num-mxNum)*nMayBomb);
			            }
			        }
			        else
			        {
			            tempLand = nMayLand-nLand;
			            if( tempLand<0 ) tempLand = 0;

			            if( nMayBomb==0 )
			            {

                            percent = ((num-nSrc)<<8)*tempLand/
                                    ((num-mxNum)*nMayLand);
			            }
			            else
			            {

			                tempBomb = nMayBomb-nBomb;
			                if( tempBomb<0 ) tempBomb = 0;
			                log_b("n4 %d %d %d %d",tempBomb,tempLand,nMayBomb,nMayLand);
                            //percent = p(吃掉)*(1-(p(炸弹))(1-p(地雷))
                            //p(地雷) = 1-nLand/nMayLand
                            percent = ((num-nSrc)<<8)*tempBomb*tempLand/
                                    ((num-mxNum)*nMayBomb*nMayLand);
			            }
			        }
			    }
			}
			else
			{
				assert( pDst->pLineup->type>SILING );
				num = aLiveTypeAll[dst];
				mxNum = aLiveTypeAll[mxDstType-1];
				nSrc = aLiveTypeAll[src];
				nSrc = (nSrc>num)?num:nSrc;
                nSrc = (src>=mxDstType)?nSrc:mxNum;
                log_b("num %d max %d mxDstType %d",num,mxNum,mxDstType);
                log_b("src %s %d",aTypeName[src],nSrc);

                if( pDst->pLineup->isNotLand || 0==nMayLand )
                {
                    percent = ((num-nSrc)<<8)/(num-mxNum);
                }
                else
                {
                    tempLand = nMayLand-nLand;
                    if( tempLand<0 ) tempLand = 0;
                    log_b("land %d %d",nMayLand,tempLand);
                    percent = ((num-nSrc)<<8)*tempLand/((num-mxNum)*nMayLand);
                }
			}
		}
		else
		{
		    assert( !pDst->pLineup->isNotLand );
			if( pDst->pLineup->type==DILEI )
			{
				percent = 256;
			}
			else if( 0!=nMayLand)
			{
                log_b("land %d %d",nLand,nMayLand);

                if( nLand<nMayLand )
                {
                    percent = (nLand<<8)/nMayLand;
                }
                else
                {
                    percent = 256;
                }
			}
			else
			{
			    percent = 0;
			}
		}
	}
	else
	{

		aLiveTypeSum = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveAllNum;
        nMayLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayLand;
        nMayBomb = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBomb;
        nMayBombLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBombLand;

        nBomb = 2-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN];
        nLand = 3-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[DILEI];
		if( pDst->pLineup->type!=DILEI)
		{
		    //log_c("dst %s %d %d",aTypeName[dst],pSrc->pLineup->iDir,pDst->pLineup->iDir);
			assert( pDst->pLineup->type>SILING );
			if( pSrc->pLineup->type==DARK )
			{

				num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
				mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
				nDst = aLiveTypeAll[dst-1]-aLiveTypeSum[dst-1];
				nTemp = aLiveTypeAll[dst]-aLiveTypeSum[dst];
	            log_b("num %d max1 %d",num,mxNum);
	            log_b("dst %s %d",aTypeName[dst],nDst);
				nDst = (nDst>num)?num:nDst;
				nDst = (nDst>nTemp)?nTemp:nDst;
				if( pSrc->pLineup->isNotBomb )
				{
				    percent = ((nDst-mxNum)<<8)/(num-mxNum);
				}
				else
				{
				    log_b("nMayBomb nMayLand nLand %d %d %d ",nMayBomb,nMayLand,nLand);
				    nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;
				    if( 0==nMayBomb )
				    {
				        percent = ((nDst-mxNum)<<8)/(num-mxNum);
				    }
				    else
				    {
                        tempBomb = nMayBomb-nBomb;
                        if( tempBomb<0 ) tempBomb = 0;
                        log_b("nMayBomb nBomb %d %d ",nMayBomb,nBomb);
                        percent = ((nDst-mxNum)<<8)*tempBomb/((num-mxNum)*nMayBomb);
				    }
				}
			}
			else
			{
				assert( pDst->pLineup->type>SILING );
				num = aLiveTypeAll[src];
				mxNum = aLiveTypeAll[mxSrcType-1];
				nDst = aLiveTypeAll[dst-1];
	            log_b("num %d max %d",num,mxNum);
	            log_b("dst %s %d",aTypeName[dst],nDst);
				nDst = (nDst>num)?num:nDst;
				percent = ((nDst-mxNum)<<8)/(num-mxNum);
			}
		}
		else
		{
			if( pSrc->pLineup->type==GONGB )
			{
				percent = 256;
			}
			else
			{
			    if( pSrc->pLineup->isNotBomb ) nBomb = 0;

				nSapper = 3-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB];
				num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB])+nBomb;
				mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
				percent = (nSapper<<8)/(num-mxNum);
				log_b("num %d max %d",num,mxNum);
				log_b("gongb %d",nSapper);
			}
		}
	}

	return percent;
}

int GetBombPercent(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst)
{
	int percent = 0;
	u8 *aLiveTypeSum;
	u8 *aLiveTypeAll;
	int src = pSrc->pLineup->type;
	int dst = pDst->pLineup->type;
	int mxSrcType = pSrc->pLineup->mx_type;
	int mxDstType = pDst->pLineup->mx_type;

	int nBomb = 0;
	int nLand = 0;
	int num;
	int mxNum;
	int nSrc;
	int nDst;
    int nMayLand;
    int nMayBomb;
    int nMayBombLand;
    int tempBomb;
    int tempLand;

    if( pDst->pLineup->type==ZHADAN || pSrc->pLineup->type==ZHADAN )
    {
    	return 256;
    }

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		aLiveTypeSum = pJunqi->aInfo[pDst->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pDst->pLineup->iDir].aLiveAllNum;
        nMayLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayLand;
        nMayBomb = pJunqi->aInfo[pDst->pLineup->iDir].nMayBomb;
        nMayBombLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayBombLand;

        nBomb = 2-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[ZHADAN];
        nLand = 3-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[DILEI];

		if( pDst->pLineup->type==DARK )
		{
			num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
			mxNum = (aLiveTypeAll[mxDstType-1]-aLiveTypeSum[mxDstType-1]);
			if( (aLiveTypeAll[src]-aLiveTypeSum[src])<
			        (aLiveTypeAll[src-1]-aLiveTypeSum[src-1]) )
			{
			    nSrc = 0;
			}
			else
			{
                nSrc = (aLiveTypeAll[src]-aLiveTypeSum[src])-
                        (aLiveTypeAll[src-1]-aLiveTypeSum[src-1]);
			}

			if( mxDstType>src ) nSrc = 0;
			//aLiveTypeSum[src]是明子，比如吃了38的39
			//aLiveTypeAll[src-1]-aLiveTypeSum[src-1]是暗子，比如暗司令
			//有可能吃了38的39就是40，而这里认为多出来一个暗司令
			//所以aLiveTypeAll[src-1]-aLiveTypeSum[src-1]是偏大的
			//具体场景，敌方39打兑，炸弹炸39，40吃38，此时我方39吃掉对方一个暗子，打印的值是
			//6 1 1 1 0 1
			log_b("%d %d %d %d %d %d ",src,aLiveTypeAll[src],aLiveTypeSum[src],
			        aLiveTypeAll[src-1],aLiveTypeSum[src-1],nBomb);

            log_b("bomb num %d max %d mxType %d",num,mxNum,mxDstType);
            log_b("bomb src %s %d",aTypeName[src],nSrc);

			if( pDst->pLineup->isNotBomb &&
			        (pSrc->pLineup->type!=GONGB || pDst->pLineup->index<5) )
			{
		        //用2^8代替百分比
		        percent = (nSrc<<8)/(num-mxNum);
                if( pDst->isStronghold )
                {
                    percent = percent*(nMayLand-nLand)/nMayLand;
                }
			}
			else
			{
	            //原始nMayBomb中可能含有是地雷，这个不再统计范围内
	            //可能是地雷和炸弹的棋，减去重复的，再减去地雷
	            nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;

			    if( pDst->pLineup->isNotLand )
			    {
			        if( nMayBomb==0 )
			        {
			            percent = (nSrc<<8)/(num-mxNum);
			        }
			        else
			        {
	                    tempBomb = nMayBomb-nBomb;
	                    if( tempBomb<0 ) tempBomb = 0;
	                    if( nBomb>nMayBomb ) nBomb = nMayBomb;

	                    //percent = p(打兑)*(1-(p(炸弹)))+p(炸弹)
	                    percent = ((nSrc*tempBomb+nBomb*(num-mxNum))<<8)/
	                            (nMayBomb*(num-mxNum));
			        }

			    }
			    else
			    {

                    tempBomb = nMayBomb-nBomb;
                    tempLand = nMayLand-nLand;
                    if( tempBomb<0 ) tempBomb = 0;
                    if( tempLand<0 ) tempLand = 0;
                    if( nBomb>nMayBomb ) nBomb = nMayBomb;

                    if( nMayBomb==0 )
                    {
                        if( 0==nMayLand )
                        {
                            percent = (nSrc<<8)/(num-mxNum);
                        }
                        else
                        {
                            percent = (((nSrc+(num-mxNum))*tempLand)<<8)/
                                    ((num-mxNum)*nMayLand);
                        }
                    }
                    else
                    {
                        if( 0==nMayLand )
                        {
                            percent = ((nSrc*tempBomb+nBomb*(num-mxNum))<<8)/
                                    (nMayBomb*(num-mxNum));
                        }
                        else
                        {
                            //percent = (p(打兑)*(1-(p(炸弹)))+p(炸弹))(1-p(地雷))
                            percent = (((nSrc*tempBomb+nBomb*(num-mxNum))*tempLand)<<8)/
                                    (nMayBomb*(num-mxNum)*nMayLand);
                        }
                    }
			    }
			}
		}
		else
		{
			num = aLiveTypeAll[dst];
			mxNum = aLiveTypeAll[mxDstType-1];
			nSrc = aLiveTypeAll[src]-aLiveTypeAll[src-1];
            if( pDst->pLineup->isNotLand || 0==nMayLand )
            {
                percent = (nSrc<<8)/(num-mxNum);
            }
            else
            {
                tempLand = nMayLand-nLand;
                if( tempLand<0 ) tempLand = 0;
                percent = (nSrc<<8)*tempLand/((num-mxNum)*nMayLand);
            }
		}
	}
	else
	{
		aLiveTypeSum = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveAllNum;
        nMayBomb = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBomb;
        nMayBombLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBombLand;
        nMayLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayLand;

        nBomb = 2-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN];
        nLand = 3-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[DILEI];

        if( pDst->pLineup->type!=DILEI )
        {
            if( pSrc->pLineup->type==DARK )
            {
                num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
                mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
                nDst = (aLiveTypeAll[dst]-aLiveTypeSum[dst])-
                        (aLiveTypeAll[dst-1]-aLiveTypeSum[dst-1]);
                if( nDst<0 || mxSrcType>dst ) nDst = 0;
                log_b("bomb1 num %d max %d type %d",num,mxNum,mxSrcType);
                log_b("bomb dst %s %d",aTypeName[dst],nDst);
                if( pSrc->pLineup->isNotBomb )
                {
                    //用2^8代替百分比
                    percent = (nDst<<8)/(num-mxNum);
                }
                else
                {
                    nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;
                    if( 0==nMayBomb )
                    {
                        percent = (nDst<<8)/(num-mxNum);
                    }
                    else
                    {
                        tempBomb = nMayBomb-nBomb;
                        if( tempBomb<0 ) tempBomb = 0;
                        if( nBomb>nMayBomb ) nBomb = nMayBomb;

                        log_b("nMayBomb nBomb %d %d ",nMayBomb,nBomb);
                        //percent = p(打兑)*(1-(p(炸弹)))+p(炸弹)
                        percent = ((nDst*tempBomb+nBomb*(num-mxNum))<<8)/
                                (nMayBomb*(num-mxNum));
                        log_b("bomb percent %d ",percent);
                    }
                }
            }
            else
            {
                num = aLiveTypeAll[src];
                mxNum = aLiveTypeAll[mxSrcType-1];
                nDst = aLiveTypeAll[dst]-aLiveTypeAll[dst-1];
                log_b("bomb num %d max %d src %d",num,mxNum,src);
                log_b("bomb dst %s %d",aTypeName[dst],nDst);
                //用2^8代替百分比
                percent = (nDst<<8)/(num-mxNum);
            }
        }
        else
        {
            assert( !pSrc->pLineup->isNotBomb );
            num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB])+nBomb;
            mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
            log_b("dilei %d %d %d %d",nBomb,mxSrcType,num,mxNum);
            percent = (nBomb<<8)/(num-mxNum);
        }
	}

	return percent;
}


int GetKilledPercent(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst)
{
	int percent = 0;
	u8 *aLiveTypeSum;
	u8 *aLiveTypeAll;
	int src = pSrc->pLineup->type;
	int dst = pDst->pLineup->type;
	int mxSrcType = pSrc->pLineup->mx_type;
	int mxDstType = pDst->pLineup->mx_type;

	int nBomb = 0;
	int nLand = 0;
	int nSapper = 0;
	int num;
	int mxNum;
	int nSrc;
	int nDst;
    int nMayLand;
    int nMayBomb;
    int nMayBombLand;
    int tempBomb;
    int tempLand;
    int nTemp;

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		aLiveTypeSum = pJunqi->aInfo[pDst->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pDst->pLineup->iDir].aLiveAllNum;
        nMayLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayLand;
        nMayBomb = pJunqi->aInfo[pDst->pLineup->iDir].nMayBomb;
        nMayBombLand = pJunqi->aInfo[pDst->pLineup->iDir].nMayBombLand;

        nBomb = 2-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[ZHADAN];
        nLand = 3-pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[DILEI];

		if( pDst->pLineup->type==DARK )
		{
			num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
			mxNum = (aLiveTypeAll[mxDstType-1]-aLiveTypeSum[mxDstType-1]);
			nSrc = aLiveTypeAll[src-1]-aLiveTypeSum[src-1];
			nTemp = aLiveTypeAll[src]-aLiveTypeSum[src];
	        log_b("kill num %d max %d",num,mxNum);
	        log_b("kill dst %s %d",aTypeName[dst],nSrc);
	        log_b("src-1 %d %d",aLiveTypeAll[src-1],aLiveTypeSum[src-1]);
	        nSrc = (nSrc>num)?num:nSrc;
	        nSrc = (nSrc>nTemp)?nTemp:nSrc;
	        if( pDst->pLineup->isNotBomb &&
	                (pSrc->pLineup->type!=GONGB || pDst->pLineup->index<5)  )
	        {
	            percent = ((nSrc-mxNum)<<8)/(num-mxNum);
	        }
	        else
	        {
	            nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;
	            if( 0==nMayBomb )
	            {
	                if( 0==nMayLand || pDst->pLineup->isNotLand )
	                {
	                    percent = ((nSrc-mxNum)<<8)/(num-mxNum);
	                }
	                else
	                {
	                    tempLand = nMayLand-nLand;
	                    if( tempLand<0 ) tempLand = 0;

	                    if( pSrc->pLineup->type!=GONGB )
	                    {
	                        if( nLand>nMayLand ) nLand = nMayLand;
	                        percent = ( ((nSrc-mxNum)*tempLand+
	                                    (num-mxNum)*nLand )<<8 ) /
	                                  ((num-mxNum)*nMayLand);
	                    }
	                    else
	                    {
	                        //percent = p(kill)(1-p(地雷))
	                        percent = ((nSrc-mxNum)<<8)*tempLand/
	                                ((num-mxNum)*nMayLand);
	                    }
	                }
	            }
	            else if( pDst->pLineup->isNotLand || 0==nMayLand )
	            {
                    tempBomb = nMayBomb-nBomb;
                    if( tempBomb<0 ) tempBomb = 0;
                    //percent = p(kill)*(1-(p(炸弹))
                    //p(炸弹) = 1-nBomb/nMayBomb
                    percent = ((nSrc-mxNum)<<8)*tempBomb/((num-mxNum)*nMayBomb);
	            }
	            else
	            {
                    tempBomb = nMayBomb-nBomb;
                    tempLand = nMayLand-nLand;
                    if( tempBomb<0 ) tempBomb = 0;
                    if( tempLand<0 ) tempLand = 0;

	                //percent = p(kill)*(1-(p(炸弹))(1-p(地雷)+p(地雷)
	                if( pSrc->pLineup->type!=GONGB )
	                {
	                    if( nBomb>nMayBomb ) nBomb = nMayBomb;
	                    if( nLand>nMayLand ) nLand = nMayLand;
	                    percent = ( ((nSrc-mxNum)*tempBomb*tempLand+
	                                (num-mxNum)*nMayBomb*nLand )<<8 ) /
	                              ((num-mxNum)*nMayBomb*nMayLand);
	                }
	                else
	                {
	                    //percent = p(kill)*(1-(p(炸弹))(1-p(地雷)
	                    percent = ((nSrc-mxNum)<<8)*tempBomb*tempLand/
	                            ((num-mxNum)*nMayBomb*nMayLand);
	                }
	            }
	        }
		}
		else if( dst==DILEI )
		{
		    percent = 256;
		}
		else
		{
			num = aLiveTypeAll[dst];
			mxNum = aLiveTypeAll[mxDstType-1];
			nSrc = aLiveTypeAll[src-1];
	        log_b("kill num %d max %d",num,mxNum);
	        log_b("kill dst %s %d",aTypeName[dst],nSrc);
	        nSrc = (nSrc>num)?num:nSrc;
	        nSrc = (nSrc>mxNum)?nSrc:mxNum;
	        if( pDst->pLineup->isNotLand || 0==nMayLand )
	        {
	            percent = ((nSrc-mxNum)<<8)/(num-mxNum);
	        }
	        else
	        {
                tempLand = nMayLand-nLand;
                if( tempLand<0 ) tempLand = 0;
	            if( nLand>nMayLand ) nLand = nMayLand;
	            if(src==GONGB)
	            {
                    percent = ( ((nSrc-mxNum)*tempLand)<<8 )/
                            ((num-mxNum)*nMayLand);
	            }
	            else
	            {
                    percent = ( ((nSrc-mxNum)*tempLand+nLand*(num-mxNum))<<8 )/
                            ((num-mxNum)*nMayLand);
	            }
	        }
		}
	}
	else
	{
		aLiveTypeSum = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveTypeSum;
		aLiveTypeAll = pJunqi->aInfo[pSrc->pLineup->iDir].aLiveAllNum;
        nMayLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayLand;
        nMayBomb = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBomb;
        nMayBombLand = pJunqi->aInfo[pSrc->pLineup->iDir].nMayBombLand;

        nBomb = 2-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN];
        nLand = 3-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[DILEI];
		if( pDst->pLineup->type!=DILEI )
		{
			if( pSrc->pLineup->type==DARK )
			{
				num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB]);
				mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
				nDst = (aLiveTypeAll[dst]-aLiveTypeSum[dst]);
	            log_b("kill num %d max %d",num,mxNum);
	            log_b("kill dst %s %d mxType %d",aTypeName[dst],nDst,mxSrcType);
	            nDst = (dst>=mxSrcType)?nDst:mxNum;
	            if( pSrc->pLineup->isNotBomb )
	            {
	                percent = ((num-nDst)<<8)/(num-mxNum);
	            }
	            else
	            {
	                nMayBomb = nMayBomb+nMayLand-nMayBombLand-nLand;
	                if( 0==nMayBomb )
	                {
	                    percent = ((num-nDst)<<8)/(num-mxNum);
	                }
	                else
	                {
	                    tempBomb = nMayBomb-nBomb;
	                    if( tempBomb<0 ) tempBomb = 0;
	                    percent = ((num-nDst)<<8)* tempBomb/((num-mxNum)*nMayBomb);
	                }
	            }
			}
			else
			{
				assert( pSrc->pLineup->type>SILING );
				num = aLiveTypeAll[src];
				mxNum = aLiveTypeAll[mxSrcType-1];
				nDst = aLiveTypeAll[dst];
	            log_b("kill num %d max %d",num,mxNum);
	            log_b("kill dst %s %d",aTypeName[dst],nDst);
	            nDst = (dst>=mxSrcType)?nDst:mxNum;
	            //用2^8代替百分比
	            percent = ((num-nDst)<<8)/(num-mxNum);
			}
		}
		else
		{
			nSapper = 3-pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB];

			if( pSrc->pLineup->type==DARK )
			{
			    if( pSrc->pLineup->isNotBomb ) nBomb = 0;

				num = (aLiveTypeAll[GONGB]-aLiveTypeSum[GONGB])+nBomb;
				mxNum = (aLiveTypeAll[mxSrcType-1]-aLiveTypeSum[mxSrcType-1]);
				percent = ((num-mxNum-nSapper-nBomb)<<8)/(num-mxNum);
				log_b("kill: num %d max %d",num,mxNum);
				log_b("nSapper %d",nSapper);
			}
			else
			{
			    //工兵和炸弹进不了这个条件
				num = aLiveTypeAll[src];
				mxNum = aLiveTypeAll[mxSrcType-1];
				percent = 256;
			}
		}

	}

	return percent;
}



u8 IsPossibleEat(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst )
{
	u8 rc = 1;

	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if(pSrc->pLineup->type>=pDst->pLineup->type &&
				pDst->pLineup->type>=SILING )
		{
		    if( pDst->pLineup->isNotLand )
		    {
		        return  0;
		    }
		}
		if( pSrc->pLineup->type==GONGB  &&
			(pDst->pLineup->isNotLand && pDst->pLineup->type!=JUNQI) )
		{
			return  0;
		}
		if( pSrc->pLineup->type!=GONGB && pDst->pLineup->type==DILEI )
		{
		    return 0;
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
		if( pDst->pLineup->type==JUNQI )
		{
			return 0;
		}
		if( pDst->pLineup->type==DILEI && pSrc->pLineup->type==GONGB )
		{
		    return 0;
		}
		if( pSrc->pLineup->type==ZHADAN ) return  0;
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
		if( pSrc->pLineup->type==JUNQI ) return 0;

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
		else if( pJunqi->aInfo[pDst->pLineup->iDir].aTypeNum[ZHADAN]==2 )
		{
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
			if( pSrc->pLineup->type!=DARK )
			{
				//双方都不可能是炸弹
				if( pSrc->pLineup->mx_type>pDst->pLineup->type )
				{
				    //log_b("d1");
					return  0;
				}
				//Dst小于Src最小可能的类型
				if( pSrc->pLineup->type<pDst->pLineup->type )
				{
				    //log_b("d2 %d %d",pSrc->pLineup->type,pDst->pLineup->type);
					return 0;
				}
			}
			else if( pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN]==2 )
			{
                if( pSrc->pLineup->mx_type>pDst->pLineup->type )
                {
                    return  0;
                }
			}
		}
		//只有炸弹才能和地雷军旗打兑
		if( ( pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[ZHADAN]==2 ||
				pSrc->pLineup->isNotBomb ) && (pDst->pLineup->type==DILEI) )
		{
			return  0;
		}
		if( pDst->pLineup->type==JUNQI )
		{
		    return  0;
		}
	}

	return rc;
}

//int SpecialCase(
//	Junqi *pJunqi,
//	BoardChess *pSrc,
//	BoardChess *pDst)
//{
//	int rc = 0;
//	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
//	{
//		if( pSrc->pLineup->type==SILING )
//		{
//			if( pDst->pLineup->isNotBomb )
//			{
//				return 1;
//			}
//		}
//	}
//	else
//	{
//		if( pDst->pLineup->type==SILING )
//		{
//			if( pSrc->pLineup->isNotBomb )
//			{
//				return 1;
//			}
//		}
//	}
//
//	return rc;
//}

void AdjustMovePercent(
        Junqi *pJunqi,
        BoardChess *pSrc,
        BoardChess *pDst )
{
    int iDir = pSrc->pLineup->iDir;
    MoveList *p;
    int sum;
    int nLeftBomb;


    assert( pJunqi->pMoveList!=NULL );
    if( pDst->type==NONE )
    {
        return;
    }
    if( (pSrc->pLineup->iDir&1)!=(ENGINE_DIR&1) )
    {
        nLeftBomb = 2-pJunqi->aInfo[iDir].aTypeNum[ZHADAN];
        if( pDst->pLineup->nEat>1 &&
            !pSrc->pLineup->isNotBomb &&
            nLeftBomb>0 )
        {
            p = pJunqi->pMoveList->pPre;
            if( p->move.result==KILLED &&
                p->pPre->move.result==BOMB )
            {
                sum = p->pPre->percent + p->percent;

                p->percent = sum>>2;
                p->pPre->percent = sum-p->percent;

                if( nLeftBomb==2 )
                {
                    p->percent = sum>>2;
                    p->pPre->percent = sum-p->percent;
                }
                else
                {
                    p->pPre->percent = sum>>2;
                    p->percent = sum-p->pPre->percent;
                }

//                p->pPre->pNext = pJunqi->pMoveList;
//                p->pNext->pPre = p->pPre;
//                free(p);
            }

        }
    }
}

u8 DiscardBadMove(
        Junqi *pJunqi,
        BoardChess *pSrc,
        BoardChess *pDst)
{
    u8 rc = 0;
    u8 iDir;

    if( pSrc->type==GONGB )
    {
        if( pDst->index<20 && pDst->isSapperPath )
        {
            return 1;
        }
    }
    else if( pSrc->type!=DARK )
    {
        if( (pSrc->pLineup->iDir%2)!=(ENGINE_DIR%2) )
        {
            if( pDst->type!=NONE &&
                !pDst->pLineup->isNotLand &&
                ( pSrc->pLineup->type<LVZH ||
                   pDst->pLineup->type<TUANZH ) &&
                pDst->pLineup->type!=JUNQI )
            {
                return 1;
            }

        }
        else
        {
            if(  pDst->type!=NONE &&
                 !pDst->pLineup->isNotLand &&
                 pSrc->pLineup->type<LVZH &&
                  pSrc->pLineup->type!=ZHADAN )
            {
                return 1;
            }
            else if( pSrc->pLineup->type==ZHADAN && pDst->isStronghold )
            {
                return 1;
            }
        }
    }

    if( pJunqi->cnt!=1 )
    {
        if( pJunqi->eSearchType==SEARCH_LEFT )
        {
            if( pDst->type!=NONE )
            {
                iDir = pDst->pLineup->iDir;
                if( pJunqi->myTurn!=iDir && iDir!=((pJunqi->myTurn+3)&3) )
                {
                    return 1;
                }
            }
        }
        else if( pJunqi->eSearchType==SEARCH_RIGHT )
        {
            if( pDst->type!=NONE )
            {
                iDir = pDst->pLineup->iDir;
                if( pJunqi->myTurn!=iDir && iDir!=((pJunqi->myTurn+1)&3) )
                {
                    return 1;
                }
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
	int aPercent[3] = {0};
	int bShowFlag;
	assert( pSrc->type!=NONE );

	if( pDst->type!=NONE )
	{
	    bShowFlag = pJunqi->aInfo[pDst->iDir].bShowFlag;
	}

	if( DiscardBadMove(pJunqi,pSrc,pDst) )
	{
	    return;
	}


//	if(pSrc->point.x==10&&pSrc->point.y==2&&
//	        pDst->point.x==15&&pDst->point.y==6)
//	{
//	    log_a("sd");
//	}
//    static int jj=0;
//    jj++;
//    if(jj==31313)
//    log_c("test");
//    log_c("jj %d",jj);
//
//    log_c("add %d %d %d %d",pSrc->point.x,pSrc->point.y,
//            pDst->point.x,pDst->point.y);
//	log_c("dst %d",pDst->type);
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
			InsertMoveList(pJunqi,pSrc,pDst,&temp,256);
			break;
		case EAT:
            if( !IsPossibleEat(pJunqi,pSrc,pDst) )
            {
            	continue;
            }

        	if( pDst->isStronghold && ( pDst->type==JUNQI ||
        	       ( !bShowFlag && pSrc->pLineup->iDir%2==ENGINE_DIR%2 ) )  )
        	{
        		aPercent[0] = AddJunqiMove(pJunqi,pSrc,pDst,&temp);
        		if( 0==aPercent[0] ) continue;
        	}
        	else
        	{
        		log_b("per eat");
        		aPercent[0] = GetEatPercent(pJunqi,pSrc,pDst);
        		if( 0==aPercent[0] ) continue;
        		log_b("per %d",aPercent[0]);
        		log_b("eat %d %d %d %d",pSrc->point.x,pSrc->point.y,
        				pDst->point.x,pDst->point.y);
        		InsertMoveList(pJunqi,pSrc,pDst,&temp,aPercent[0]);

        	}
			break;
		case BOMB:
            if( !IsPossibleBomb(pJunqi,pSrc,pDst) )
            {
            	continue;
            }
        	//if( pDst->isStronghold )
            if( pDst->isStronghold && ( pDst->type==JUNQI ||
                   ( !bShowFlag && pSrc->pLineup->iDir%2==ENGINE_DIR%2 ) ) )
        	{
        		aPercent[1] = AddJunqiMove(pJunqi,pSrc,pDst,&temp);
        		if( 0==aPercent[1] ) continue;
        	}
        	//暂时不考虑大本营是司令的情况
        	else
        	{
        		int percent;
        		aPercent[1] = GetBombPercent(pJunqi,pSrc,pDst);
        		if( 0==aPercent[1] ) continue;
        		log_b("per %d",aPercent[1]);
        		log_b("bomb %d %d %d %d",pSrc->point.x,pSrc->point.y,
        				pDst->point.x,pDst->point.y);

        		percent = aPercent[1]-AddCommanderMove(pJunqi,pSrc,pDst,&temp,aPercent[1]);
        		log_b("spe %d",percent);
        		if( percent!=0 )
        		{
        			InsertMoveList(pJunqi,pSrc,pDst,&temp,percent);
        		}

        	}
			break;
		case KILLED:

            if( !IsPossibleKilled(pJunqi,pSrc,pDst) )
            {
            	continue;
            }
            //if( pDst->isStronghold )
            if( pDst->isStronghold && ( pDst->type==JUNQI ||
                   ( !bShowFlag && pSrc->pLineup->iDir%2==ENGINE_DIR%2 ) ) )
            {
            	aPercent[2] = 256-aPercent[0]-aPercent[1];
            	if( 0==aPercent[2] ) continue;
            }
            else
            {
				aPercent[2] = GetKilledPercent(pJunqi,pSrc,pDst);
				if( 0==aPercent[2] ) continue;
				log_b("per %d",aPercent[2]);
				log_b("Killed %d %d %d %d",pSrc->point.x,pSrc->point.y,
						pDst->point.x,pDst->point.y);
            }
            InsertMoveList(pJunqi,pSrc,pDst,&temp,aPercent[2]);
            //司令撞地雷，无论怎么样对方都是亏的，不考虑对方会下
            //AddCommanderKilled(pJunqi,pSrc,pDst,&temp);
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

	AdjustMovePercent(pJunqi,pSrc,pDst);

	if( pDst->type!=NONE )
	{
	    assert( (pSrc->pLineup->iDir&1)!=(pDst->pLineup->iDir&1) );
		log_b("percent %d %d %d",aPercent[0],aPercent[1],aPercent[2]);
		int sum = aPercent[0]+aPercent[1]+aPercent[2];

//		if( sum<251 && pDst->isStronghold )
//		{
//			for(int i=0;i<3;i++)
//			{
//				if( aPercent[i]!=0 )
//				{
//					aPercent[i] += 256-sum;
//					break;
//				}
//			}
//		}
		if( !(sum>250&&sum<=256) )
		{
			sleep(1);
			assert(sum>250&&sum<=256);
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

void ClearMoveList(Junqi *pJunqi, MoveList *pHead)
{
	MoveList *p;
	MoveList *pTmp;

	if( pHead==NULL )
	{
		return;
	}
	p = pHead->pNext;

	while(1)
	{
		if(p->isHead)
		{
			//free(p);
			//MoveNodeFree(pJunqi,p);
			memsys5Free(pJunqi,p);
			break;
		}
		else
		{
			pTmp = p;
			p = p->pNext;
			//free(pTmp);
			//MoveNodeFree(pJunqi,pTmp);
			memsys5Free(pJunqi,pTmp);
		}

	}

}

u8 IsDirectRail(
        Junqi *pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pDst )
{
    BoardChess *pSrcChess;
    BoardChess *pDstChess;
    u8 rc =0;

    pSrcChess = pSrc->pAdjList->pChess;
    pDstChess = pDst->pAdjList->pChess;

    if( pSrcChess->point.x==pDstChess->point.x )
        rc = 1;

    if( pSrcChess->point.y==pDstChess->point.y )
        rc = 1;

    if( pSrcChess->eCurveRail==pDstChess->eCurveRail && pSrcChess->eCurveRail>0 )
        rc = 1;

    if( !rc )
    {
        pDstChess->isSapperPath = 1;
    }
    else
    {
        pDstChess->isSapperPath = 0;
    }

    return rc;
}

void SearchRailPath(
        Junqi* pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pDst,
        int flag )
{
    AdjNode *p;
    BoardGraph *pVertex;
    BoardChess *pChess = pSrc->pAdjList->pChess;


    pDst->passCnt++;

    for(p=pDst->pAdjList->pNext; p!=NULL; p=p->pNext)
    {
        pVertex = &pJunqi->aBoard[p->pChess->point.x][p->pChess->point.y];

        if( pVertex->passCnt!=0 )
        {
            continue;
        }
        //顺序不能调换，IsDirectRail必须执行，记录是否是工兵特有路径
        else if( !IsDirectRail(pJunqi, pSrc, pVertex) && pChess->type!=GONGB  )
        {
            continue;
        }
        else if( p->pChess->type!=NONE )
        {
            if( (p->pChess->pLineup->iDir&1)!=(pChess->pLineup->iDir&1) )
            {
                pVertex->passCnt++;
                if( !flag )
                {
                    AddMoveToList(pJunqi, pChess, p->pChess);

//                    log_a("dir %d %d",p->pChess->iDir,pChess->iDir);
//                    log_a("dst %d %d %d %d",pChess->point.x,pChess->point.y,
//                            p->pChess->point.x,p->pChess->point.y);
                }
                else
                {
                    AddMoveToHash(pJunqi, pChess, p->pChess);
                }
            }
            continue;
        }
        else
        {

            if( !flag )
            {
                AddMoveToList(pJunqi, pChess, p->pChess);
//                log_a("path %d %d %d %d",pChess->point.x,pChess->point.y,
//                        p->pChess->point.x,p->pChess->point.y);
            }
            SearchRailPath(pJunqi, pSrc, pVertex,flag);
        }
    }

}


void SearchMovePath(
        Junqi* pJunqi,
        BoardChess *pSrc,
        int flag )
{

    BoardGraph *pVertex;
    BoardChess *pNbr;
    BoardGraph *pNbrVertex;

    int i,x,y;

    ClearPassCnt(pJunqi);

    if( pSrc->isStronghold )
    {
        return;
    }
    else if( pSrc->type==NONE )
    {
        return;
    }
    else if( pSrc->type==DILEI || pSrc->type==JUNQI )
    {
        return;
    }

    if( pSrc->isRailway )
    {
        pVertex = &pJunqi->aBoard[pSrc->point.x][pSrc->point.y];

        SearchRailPath(pJunqi, pVertex, pVertex, flag);
    }
    for(i=0; i<9; i++)
    {
        if( i==4 ) continue;
        x = pSrc->point.x+1-i%3;
        y = pSrc->point.y+i/3-1;

        if( x<0||x>16||y<0||y>16 ) continue;

        if( pJunqi->aBoard[x][y].pAdjList )
        {
            pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;
            pNbrVertex = &pJunqi->aBoard[pNbr->point.x][pNbr->point.y];
            if( pNbrVertex->passCnt )
            {
                continue;
            }
            else if( pNbr->isCamp && pNbr->type!=NONE && !flag )
            {
                continue;
            }
            else if( pNbr->type!=NONE && (pNbr->pLineup->iDir&1)==(pSrc->pLineup->iDir&1) )
            {
                continue;
            }
            pNbr->isSapperPath = 0;

            if( pSrc->isCamp || pNbr->isCamp )
            {
                if( !flag )
                {
                    AddMoveToList(pJunqi, pSrc, pNbr);
    //                log_a("nbr1 %d %d %d %d",pSrc->point.x,pSrc->point.y,
    //                        pNbr->point.x,pNbr->point.y);
                }
                else if( pNbr->type!=NONE )
                {
                    AddMoveToHash(pJunqi, pSrc, pNbr);
                }
            }
            //非斜相邻
            else if( pNbr->point.x==pSrc->point.x || pNbr->point.y==pSrc->point.y)
            {
                if( !flag )
                {
                    AddMoveToList(pJunqi, pSrc, pNbr);
    //                log_a("nbr2 %d %d %d %d",pSrc->point.x,pSrc->point.y,
    //                        pNbr->point.x,pNbr->point.y);
                }
                else if( pNbr->type!=NONE )
                {
                    AddMoveToHash(pJunqi, pSrc, pNbr);
                }
            }

        }
    }
}

MoveList *GenerateMoveList(Junqi* pJunqi, int iDir)
{
    int i;
    BoardChess *pSrc;
    ChessLineup *pLineup;
    pJunqi->searche_num[0]++;
    pJunqi->pMoveList = NULL;
    for(i=0;  i<30; i++)
    {
        pLineup = &pJunqi->Lineup[iDir][i];
        if( pLineup->bDead || pLineup->type==NONE )
        {
            continue;
        }
        pSrc = pLineup->pChess;
        SearchMovePath(pJunqi,pSrc,0);
    }
    return pJunqi->pMoveList;
}
