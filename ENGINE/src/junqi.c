/*
 * junqi.c
 *
 *  Created on: Aug 17, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "path.h"
#include "search.h"

extern Junqi* gJunqi;

//>=司令：1，>=军长：2，>=师长：4，>=旅长：6
const u8 aMaxTypeNum[14] =
{
	0,0,1,3,2,1,2,4,6,8,10,13,16,19
};

Junqi *JunqiOpen(void)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));
	gJunqi = pJunqi;
	pthread_mutex_init(&pJunqi->mutex, NULL);
	return pJunqi;
}

void InitReplyLineup(Junqi* pJunqi, u8 *data)
{
//	int i,j;
//	u8 aRpLineup[4][30];
//
//	for(j=0; j<4; j++)
//	{
//		for(i=0; i<30; i++)
//		{
//			aRpLineup[j][i] = data[30*j+i];
//		}
//	}
}

void InitLineup(Junqi* pJunqi, u8 *data, u8 isInit)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	int i,j,k;

	data = (u8*)&pHead[1];
	k=4;
	for(j=0; j<4; j++)
	{
		for(i=0; i<30; i++)
		{
			if( data[j]==1 )
			{
				pJunqi->Lineup[j][i].type = data[k++];
			}
			else //if(!isInit)
			{
				pJunqi->Lineup[j][i].type = DARK;
			}
		}

	}
}


void SetBoardCamp(Junqi *pJunqi, enum ChessDir dir, int i)
{
	if(i==6||i==8||i==12||i==16||i==18)
	{
		pJunqi->ChessPos[dir][i].isCamp = 1;
		pJunqi->ChessPos[dir][i].type = NONE;
		pJunqi->Lineup[dir][i].type = NONE;
	}
	else if(i==26||i==28)
	{
		pJunqi->ChessPos[dir][i].isStronghold = 1;
	}
}

void SetBoardRailway(Junqi *pJunqi, enum ChessDir dir, int i)
{
	if(i<25)
	{
		if( (i/5==0||i/5==4) || ((i%5==0||i%5==4)) )
		{
			pJunqi->ChessPos[dir][i].isRailway = 1;
		}
	}
}


void SetChess(Junqi *pJunqi, enum ChessDir dir)
{
	enum ChessType iType;
	int i;

    //从方阵的左上角从上往下，从左往右遍历
	for(i=0;i<30;i++)
	{
		iType = pJunqi->Lineup[dir][i].type;

		assert( iType>=NONE && iType<=GONGB );
        // 下家最右侧横坐标为0
		// 对家最上面纵坐标为0
		switch(dir)
		{
		case HOME:
			pJunqi->ChessPos[dir][i].point.x = 10-i%5;
			pJunqi->ChessPos[dir][i].point.y = 11+i/5;
			break;
		case RIGHT:
			pJunqi->ChessPos[dir][i].point.x = 5-i/5;
			pJunqi->ChessPos[dir][i].point.y = 10-i%5;
			break;
		case OPPS:
			pJunqi->ChessPos[dir][i].point.x = 6+i%5;
			pJunqi->ChessPos[dir][i].point.y = 5-i/5;
			break;
		case LEFT:
			pJunqi->ChessPos[dir][i].point.x = 11+i/5;
			pJunqi->ChessPos[dir][i].point.y = 6+i%5;
			break;
		default:
			assert(0);
			break;
		}

		pJunqi->ChessPos[dir][i].pLineup = &pJunqi->Lineup[dir][i];
		pJunqi->ChessPos[dir][i].type = pJunqi->Lineup[dir][i].type;
		pJunqi->ChessPos[dir][i].index = i;
		pJunqi->ChessPos[dir][i].iDir = dir;
		pJunqi->Lineup[dir][i].iDir = dir;
		pJunqi->Lineup[dir][i].pChess = &pJunqi->ChessPos[dir][i];
		pJunqi->Lineup[dir][i].bDead = 0;
		pJunqi->Lineup[dir][i].index = i;
		pJunqi->Lineup[dir][i].mx_type= SILING;
		if( i<5 )   pJunqi->Lineup[dir][i].isNotBomb = 1;
		if( i<20 )  pJunqi->Lineup[dir][i].isNotLand = 1;
		SetBoardCamp(pJunqi, dir, i);
		SetBoardRailway(pJunqi, dir, i);
	}
}

void CreatGraphVertex(Junqi *pJunqi, BoardChess *pChess)
{
	AdjNode *pNode;
	BoardGraph *pVertex;
	pNode = (AdjNode *)malloc(sizeof(AdjNode));
	pNode->pChess = pChess;
	pNode->pNext = NULL;
	pVertex = &pJunqi->aBoard[pChess->point.x][pChess->point.y];

	assert( pVertex->pAdjList==NULL );
	pVertex->pAdjList = pNode;
}

void InitNineGrid(Junqi *pJunqi)
{
	int i;
	for(i=0; i<9; i++)
	{
		pJunqi->NineGrid[i].type = NONE;
		pJunqi->NineGrid[i].point.x = 10-(i%3)*2;
		pJunqi->NineGrid[i].point.y = 6+(i/3)*2;
		pJunqi->NineGrid[i].isRailway = 1;
		pJunqi->NineGrid[i].isNineGrid = 1;
		CreatGraphVertex(pJunqi,&pJunqi->NineGrid[i]);

	}

}

void AddAdjNode(
		Junqi *pJunqi,
		BoardGraph *pVertex,
		int i,
		int j,
		u8 isNineGrid
		)
{
	u8 newFlag;
	AdjNode *pNew;
	AdjNode *pNode = pJunqi->aBoard[i][j].pAdjList;

	if( pNode!=NULL )
	{
		if(isNineGrid)
		{
			newFlag = pNode->pChess->isNineGrid;
		}
		else
		{
			newFlag = pNode->pChess->isRailway;
		}

		if( newFlag )
		{
			pNew = (AdjNode*)malloc(sizeof(AdjNode));
			pNew->pChess =pNode->pChess;
			pNew->pNext = pVertex->pAdjList->pNext;
			pVertex->pAdjList->pNext = pNew;
		}
	}
}

/*
 * 添加4个角上的铁路邻居
 */
void AddSpcNode(Junqi *pJunqi)
{
	BoardGraph *pVertex;
	int i,j;

	i = 10;
	j = 11;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, j, i, 0);
	pVertex = &pJunqi->aBoard[j][i];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 6;
	j = 11;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, i-1, j-1, 0);
	pVertex = &pJunqi->aBoard[i-1][j-1];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 6;
	j = 5;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, j, i, 0);
	pVertex = &pJunqi->aBoard[j][i];
	AddAdjNode(pJunqi, pVertex, i, j, 0);

	i = 11;
	j = 6;
	pVertex = &pJunqi->aBoard[i][j];
	AddAdjNode(pJunqi, pVertex, i-1, j-1, 0);
	pVertex = &pJunqi->aBoard[i-1][j-1];
	AddAdjNode(pJunqi, pVertex, i, j, 0);
}

void InitBoardGraph(Junqi *pJunqi)
{
	int i,j;
	BoardGraph *pVertex;

	for(i=0; i<17; i++)
	{
		for(j=0; j<17; j++)
		{
			pVertex = &pJunqi->aBoard[i][j];
			if( pVertex->pAdjList!=NULL )
			{
				assert( pVertex->pAdjList->pChess!=NULL );
				if( pVertex->pAdjList->pChess->isRailway )
				{
					AddAdjNode(pJunqi, pVertex, i+1, j, 0);
					AddAdjNode(pJunqi, pVertex, i-1, j, 0);
					AddAdjNode(pJunqi, pVertex, i, j+1, 0);
					AddAdjNode(pJunqi, pVertex, i, j-1, 0);
					if( pVertex->pAdjList->pChess->isNineGrid )
					{
						AddAdjNode(pJunqi, pVertex, i+2, j, 1);
						AddAdjNode(pJunqi, pVertex, i-2, j, 1);
						AddAdjNode(pJunqi, pVertex, i, j+2, 1);
						AddAdjNode(pJunqi, pVertex, i, j-2, 1);
					}
				}
			}
		}
	}
	AddSpcNode(pJunqi);

#if 0//测试邻接表初始化是否正确
	i = 6;
	j = 8;
	pVertex = &pJunqi->aBoard[i][j];
	AdjNode *p;
	for(p = pVertex->pAdjList;p!=NULL;p=p->pNext)
	{
		log_b("%d %d",p->pChess->point.x,p->pChess->point.y);

	}
#endif
}

void InitCurveRail(Junqi *pJunqi)
{
	int i,j;
	for(i=0; i<4; i++)
	{
		for(j=0; j<30; j++)
		{
			if(j%5==4)
			{
				pJunqi->ChessPos[i][j].eCurveRail = i+RAIL1;
				pJunqi->ChessPos[(i+1)%4][j-4].eCurveRail = i+RAIL1;
			}
		}
	}
	/////拐弯铁路测试//////////
#if 0
//	i = 6;
//	j = 11;
//	BoardGraph *pVertex = &pJunqi->aBoard[i+1][j+1];
//    log_a("rail %d",pVertex->pAdjList->pChess->eCurveRail);
#endif

}

void DestroyAllChess(Junqi *pJunqi, int iDir)
{
	int i,j;
	BoardChess *pChess;

	for(i=0; i<17; i++)
	{
		for(j=0; j<17; j++)
		{
			if( pJunqi->aBoard[i][j].pAdjList )
			{
				pChess = pJunqi->aBoard[i][j].pAdjList->pChess;
				assert( pChess );
				if( pChess && pChess->pLineup && pChess->pLineup->iDir==iDir )
				{
					pChess->type = NONE;
				}
			}
		}
	}

	pJunqi->aInfo[iDir].bDead = 1;
	if( pJunqi->aInfo[(iDir+2)%4].bDead==1 )
	{
		if( !pJunqi->bSearch )
		{
			pJunqi->bStart = 0;
		}
	}

}

void ChessTurn(Junqi *pJunqi)
{
	pJunqi->eTurn = (pJunqi->eTurn+1)%4;
	//下家阵亡
	if( pJunqi->aInfo[pJunqi->eTurn].bDead )
	{
		pJunqi->eTurn = (pJunqi->eTurn+1)%4;
		//下下家阵亡
		if( pJunqi->aInfo[pJunqi->eTurn].bDead )
		{
			pJunqi->eTurn = (pJunqi->eTurn+1)%4;
		}
	}

}

void IncJumpCnt(Junqi *pJunqi, int iDir)
{
	int cntJump;
	pJunqi->aInfo[iDir].cntJump++;
	cntJump = pJunqi->aInfo[iDir].cntJump;

	if( cntJump==5 )
	{
		DestroyAllChess(pJunqi, iDir);
	}
}


int aseertChess(BoardChess *pChess)
{
	int rc = 1;
	if( pChess && pChess->type!=NONE )
	{
		if( pChess!=pChess->pLineup->pChess )
		{
		    log_c("ss1");
			rc = 0;
		}
		else if( pChess->pLineup->type!=pChess->type )
		{
		    log_c("ss2 %d %d %d %d",pChess->pLineup->type,pChess->type,
		            pChess->point.x,pChess->point.y);
		    rc = 0;
		}
	}
	return rc;
}

void GetTypeNum(u8 *aBombNum, u8 *aTypeNum, u8 *aTypeNumSum)
{
	int i;
	int sum = 0;
	int sum1 = 0;
	int nBomb = 0;
	int sub;

	for(i=SILING; i<=GONGB; i++)
	{
		sum += aTypeNum[i];
		sum1 += aBombNum[i];
		aTypeNumSum[i] = sum - ((sum1<(2-aTypeNum[ZHADAN]))?sum1:(2-aTypeNum[ZHADAN]));
		//高于当前级别的数量已超出最大值，那么超出的部分必定是炸弹
		if( (sub=sum-aMaxTypeNum[i])>nBomb )
		{
			nBomb = sub;
		}
	}
	aTypeNum[ZHADAN] += nBomb;
	assert( aTypeNum[ZHADAN]<=2 );
}

void GetLiveTypeAll(
		u8 *aDeadType,
		u8 *aLiveAllNum,
		u8 *aBombNum,
		int nBombType)
{
	int i;
	int sum = 0;
	int sum1 = 0;
    int nBomb = 0;

	for(i=SILING; i<=GONGB; i++)
	{
		sum += aDeadType[i];
        sum1 += aBombNum[i];
        nBomb = (sum1<(2-nBombType))?sum1:(2-nBombType);
		aLiveAllNum[i] = aMaxTypeNum[i]-sum+nBomb;
	}

    for(i=GONGB; i>SILING; i--)
    {
        if( aLiveAllNum[i]<aLiveAllNum[i-1] )
        {
            aLiveAllNum[i-1] = aLiveAllNum[i];
        }
    }

}

void GetLiveTypeSum(
		u8 *aLiveTypeSum,
		u8 *aLiveTpyeNum,
		u8 *aLiveAllNum )
{
	int i;
	int sum = 0;
	int sub = 0;
	for(i=SILING; i<=GONGB; i++)
	{
		sum += aLiveTpyeNum[i];
		aLiveTypeSum[i] = sum;
	}

    for(i=GONGB; i>SILING; i--)
    {
        sub = (aLiveAllNum[i]-aLiveTypeSum[i])-(aLiveAllNum[i-1]-aLiveTypeSum[i-1]);
        if( sub<0 )
        {
            aLiveTypeSum[i-1] -= sub ;
        }
    }
}

int GetMaxType(int mx_type, int type, u8 *aTypeNumSum)
{
	enum ChessType tmp;

	tmp = mx_type;
	while( tmp<type )
	{
		//大于等于tmp的数量已经到最大值，所以mx_type已经不可能是tmp
		//那这里为什么不退出而要继续搜索呢，这里还是举个例子
		//司令死掉，有3个子吃掉37，而大于等于39的子并没有到最大数量
		//那么是否可以判断最大就是39了呢，显然不是，后面发现，大于等于
		//38的数量也到了最大值，所以当前这个子最大只可能是37
		if( aTypeNumSum[tmp]>=aMaxTypeNum[tmp] )
		{
			mx_type = ++tmp;
		}
		else
		{
			++tmp;
		}
	}
	return mx_type;
}

void RecordAdjustInfo(Junqi *pJunqi, int iDir, int index, int type)
{
    Engine *pEngine = pJunqi->pEngine;
    PositionList *pHead;
    PositionData *pStorage;
    PositionList *pTail;

    pHead = pEngine->pPos;
    pTail = pHead->pPre;
    pStorage = &pTail->data;

    pStorage->xExtraInfo.adjustFlag = 1;
    pStorage->xExtraInfo.adjusrDir = iDir;
    pStorage->xExtraInfo.adjusrIndex = index;
    pStorage->xExtraInfo.saveType = type;
}
//待优化
void AdjustMaxType(Junqi *pJunqi, int iDir)
{
	int i;
	ChessLineup *pLineup;
	u8 *aTypeNum = pJunqi->aInfo[iDir].aTypeNum;
	u8 *aLiveTypeSum = pJunqi->aInfo[iDir].aLiveTypeSum;
	u8 aLiveTypeNum[14] = {0};
	u8 *aLiveAllNum = pJunqi->aInfo[iDir].aLiveAllNum;
	u8 aBombNum[14] = {0};
	u8 aTypeNumSum[14] = {0};
	u8 aDeadType[14] = {0};
	int nBomb;
	enum ChessType tmp;

	pJunqi->aInfo[iDir].nMayLand = 0;
	pJunqi->aInfo[iDir].nMayBombLand = 0;
	pJunqi->aInfo[iDir].nMayBomb = 0;
	memset(aTypeNum, 0, 14);
	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->type==NONE )
		{
			continue;
		}
        if( !pLineup->bDead )
        {
            if( !pLineup->isNotLand )
            {
                pJunqi->aInfo[iDir].nMayLand++;
                if( !pLineup->isNotBomb )
                {
                    pJunqi->aInfo[iDir].nMayBombLand++;
                    pJunqi->aInfo[iDir].nMayBomb++;
                }
            }
            else if( !pLineup->isNotBomb )
            {
                pJunqi->aInfo[iDir].nMayBomb++;
            }
        }
		if( pLineup->type==DARK )
		{
		    continue;
		}
		//疑似地雷的棋，不要把pLineup->type统计进去
		if( pLineup->index>=20 && !pLineup->isNotLand )
		{
			if( pLineup->type!=DILEI )
				continue;
		}

		if( pLineup->bDead )
		{
			aDeadType[pLineup->type]++;
		}
		else
		{
			aLiveTypeNum[pLineup->type]++;
		}

		//计算该子类型的总和
		aTypeNum[pLineup->type]++;

		//计算该子暗打兑的数量，打兑当中有些是炸弹，需要在后续判断排除
		if( pLineup->bBomb )
		{
			aBombNum[pLineup->type]++;
		}
	}

	nBomb = aTypeNum[ZHADAN];
    //工兵大于3，说明多余的飞了炸
    if( aTypeNum[GONGB]>3 )
    {
        int sub;
        log_b("gongb zhad %d %d",aTypeNum[GONGB], aTypeNum[ZHADAN]);
        sub = aTypeNum[GONGB]-3;
        aTypeNum[ZHADAN] += sub;
        aTypeNum[GONGB] = 3;
        aDeadType[GONGB] -= sub;
        assert( aTypeNum[ZHADAN]<=2 );
    }

	GetLiveTypeAll(aDeadType,aLiveAllNum,aBombNum,nBomb);

	//获取某个级别以上的数量总和，保存在aTypeNumSum里
	//这里是先把aTypeNumSum都算好，因为aTypeNumSum是固定的
	//如果后面再循环中算则重复了
	GetTypeNum(aBombNum,aTypeNum,aTypeNumSum);

	GetLiveTypeSum(aLiveTypeSum,aLiveTypeNum,aLiveAllNum);
    //这里先计算好暗子的最大可能性
	tmp = GetMaxType(SILING, GONGB, aTypeNumSum);

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		//NONE ~ SILING
		if( pLineup->type<SILING && pLineup->type!=DARK )
		{
			continue;
		}
		//当现在估计的子力比之前算的小的话才更新
		if( pLineup->type==DARK )
		{
			if( pLineup->mx_type<tmp )
			{
				pLineup->mx_type = tmp;
			}
		}
		else
		{
			assert( pLineup->type>=SILING );

			//这里计算疑似地雷的棋，举个例子，对方司令已经死了
			//此时38撞雷，我们还不能判断是地雷，也可能是39，
			//如果又有另一个子吃了38，那么可以判断是地雷
			//这里比当前棋级别大的数量已经为最大值
			//后2排的pLineup->type是没有统计到aTypeNumSum里的，所以可以断定为地雷

            //后2排疑似地雷的type不会统计到aTypeNumSum里
            if( pLineup->index>=20 && !pLineup->isNotLand )
            {
                pLineup->mx_type = tmp;
                if( pLineup->type!=DILEI && pLineup->type<tmp )
                {
                    RecordAdjustInfo(pJunqi,iDir,pLineup->index,pLineup->type);
                    pLineup->type = DILEI;
                    if( !pLineup->bDead )
                    {
                        pLineup->pChess->type = DILEI;
                    }

                    aTypeNum[DILEI]++;
                }
            }
            else
            {
                //这里计算吃过子的棋的最大可能
                pLineup->mx_type = GetMaxType(pLineup->mx_type,
                        pLineup->type, aTypeNumSum);
            }

		}

	}

}

void JudgeIfBomb(
		Junqi *pJunqi,
		BoardChess *pSrc,
		BoardChess *pDst,
		MoveResultData* pResult
		)
{
	if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
	{
		if( pDst->pLineup->type==DARK && pDst->pLineup->index>=5 )
		{
			pDst->pLineup->bBomb = 1;
		}

		if( (pSrc->pLineup->type==SILING) && !(pResult->extra_info&0x04))
		{
			pDst->pLineup->type = ZHADAN;
		}
		else if( pSrc->pLineup->type!=ZHADAN )
		{
			pDst->pLineup->type = pSrc->pLineup->type;
			pDst->pLineup->mx_type = pSrc->pLineup->type;
		}

	}
	else
	{
		if( pSrc->pLineup->type==DARK && pSrc->pLineup->index>=5 )
		{
			pSrc->pLineup->bBomb = 1;
		}

		if( ((pDst->pLineup->type==SILING) && !(pResult->extra_info&0x02)) ||
				pDst->pLineup->type==DILEI )
		{
			pSrc->pLineup->type = ZHADAN;
		}
		else if( pDst->pLineup->type!=ZHADAN )
		{
			pSrc->pLineup->type = pDst->pLineup->type;
			pSrc->pLineup->mx_type = pDst->pLineup->type;
		}
	}
}

void PlayResult(
		Junqi *pJunqi,
		BoardChess *pSrc,
		BoardChess *pDst,
		MoveResultData* pResult
		)
{

	int type = pResult->result;
	BoardChess *pJunqiChess;
	BoardPoint p;
	int iDir1,iDir2;
	u8 bLandMove = 0;

	assert( aseertChess(pSrc) );
	assert( aseertChess(pDst) );

	//assert( pJunqi->Lineup[0][24].type!=DILEI );//测试用
//    log_c("play %d %d %d %d type %d",pSrc->point.x,pSrc->point.y,
//            pDst->point.x,pDst->point.y,type);
//    log_c("src %d dst %d",pSrc->type,pDst->type);
	iDir1 = pSrc->pLineup->iDir;
	if( type!=MOVE )
	{
		iDir2 = pDst->pLineup->iDir;
	}
	//在模拟行棋时，为了搜索数量，己方的司令信息并没有记录在extra_info
	if( pResult->extra_info&0x02 )
	{

		p.x = pResult->junqi_src[0];
		p.y = pResult->junqi_src[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->type = JUNQI;
        pJunqiChess->pLineup->isNotBomb = 1;
        pJunqiChess->pLineup->isNotLand = 1;
		pSrc->pLineup->type = SILING;
		pJunqi->aInfo[iDir1].bShowFlag |= 2;
	}
	if( pResult->extra_info&0x04 )
	{

		p.x = pResult->junqi_dst[0];
		p.y = pResult->junqi_dst[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->pLineup->isNotBomb = 1;
		pJunqiChess->pLineup->isNotLand = 1;
		pJunqiChess->type = JUNQI;
		pDst->pLineup->type = SILING;
		pJunqi->aInfo[iDir2].bShowFlag |= 2;
	}

	if( pDst->isStronghold && ((pResult->extra_info&1)==0) )
	{
	    if( !pJunqi->aInfo[pDst->iDir].bDead )
	    {
            //假旗被挖后，另外一个大本营只能是军旗
            if( pDst->index==26 )
            {
                pJunqi->Lineup[pDst->iDir][28].type = JUNQI;
                pJunqi->ChessPos[pDst->iDir][28].type = JUNQI;
            }
            else
            {
                pJunqi->Lineup[pDst->iDir][26].type = JUNQI;
                pJunqi->ChessPos[pDst->iDir][26].type = JUNQI;
            }
            pJunqi->aInfo[pDst->iDir].bShowFlag |= 1;
	    }
	}

	if( pSrc->pLineup->index>=20 )
	{
	    bLandMove = 1;
		pSrc->pLineup->isNotLand = 1;
	}

	if( pSrc->pLineup->type==DARK )
	{
		if( !IsEnableMove(pJunqi, pSrc, pDst) )
		{
			pSrc->pLineup->type = GONGB;
			pSrc->pLineup->mx_type = GONGB;
			pSrc->pLineup->isNotBomb = 1;
			pSrc->pLineup->isNotLand = 1;
			bLandMove = 1;
		}
	}

	if( type==EAT || type==BOMB )
	{
		pDst->pLineup->bDead = 1;

		assert(pDst->type!=NONE);
		if( type==BOMB )
		{

			pDst->type = NONE;
			pSrc->pLineup->bDead = 1;
            if( pSrc->pLineup->type==SILING )
            {
            	pJunqi->aInfo[iDir1].bShowFlag |= 2;
            }
            if( pDst->pLineup->type==SILING )
            {
            	pJunqi->aInfo[iDir2].bShowFlag |= 2;
            }

			JudgeIfBomb(pJunqi, pSrc, pDst, pResult);
		}
		else
		{
			if( pSrc->pLineup->index>=5 )
			{
				pSrc->pLineup->isNotBomb = 1;
			}

			if( pSrc->pLineup->iDir%2!=ENGINE_DIR%2 )
			{
				if( pDst->type!=DILEI && pDst->type!=JUNQI &&
						(pDst->type<=pSrc->pLineup->type || pSrc->pLineup->type==DARK) )
				{
					//log_a("change type %d %d",pSrc->pLineup->type,pDst->type);
					pSrc->pLineup->type = pDst->type-1;
				}
				else if( pDst->type==DILEI )
				{
					pSrc->pLineup->type = GONGB;
					pSrc->pLineup->mx_type = GONGB;
				}
			}
			else
			{
				if( pSrc->pLineup->type==GONGB )
				{
					pDst->pLineup->type = DILEI;
				}
				//如果是用40吃的子，对方最大39，只有对方的最大值比39大时才更新
				else if( pDst->pLineup->mx_type < pSrc->pLineup->type+1 )
				{
					pDst->pLineup->mx_type = pSrc->pLineup->type+1;
				}
			}
		}

		if( pResult->extra_info&1 )
		{
			if( type==BOMB )
			{
				pSrc->pLineup->type = ZHADAN;
			}
			pDst->pLineup->type = JUNQI;
			DestroyAllChess(pJunqi, pDst->pLineup->iDir);
		}


	}

	if( type==EAT || type==MOVE )
	{
		pDst->type = pSrc->pLineup->type;
		pDst->pLineup = pSrc->pLineup;
		pDst->pLineup->pChess = pDst;
	}

	if( type==KILLED )
	{
		pSrc->pLineup->bDead = 1;

		if( pSrc->type==GONGB && pDst->pLineup->index>=20 )
		{
			pDst->pLineup->isNotLand = 1;
		}
		if( pDst->pLineup->index>=5 )
		{
			pDst->pLineup->isNotBomb = 1;
		}


		if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
		{
			if( pDst->pLineup->type>=pSrc->type || pDst->pLineup->type==DARK )
			{
				pDst->pLineup->type = pSrc->type-1;
				pDst->type = pDst->pLineup->type;
			}
			if( pSrc->pLineup->type<=pDst->pLineup->mx_type )
			{
				assert( pDst->pLineup->index>=20 );
				pDst->pLineup->type = DILEI;
				pDst->type = DILEI;

			}
		}
		else
		{
			if( pDst->pLineup->type!=DILEI )
			{
				//如果是用40吃的子，对方最大39，只有对方的最大值比39大时才更新
				if( pSrc->pLineup->mx_type < pDst->pLineup->type+1 )
					pSrc->pLineup->mx_type = pDst->pLineup->type+1;
			}

		}

        if( pSrc->pLineup->type==SILING )
        {
        	pJunqi->aInfo[iDir1].bShowFlag |= 2;
        }

	}
	pSrc->type = NONE;

	//预测敌方最大的棋子
	if( type!=MOVE )
	{
		if( iDir1%2!=ENGINE_DIR%2 )
		{
			AdjustMaxType(pJunqi, iDir1);
		}
		else
		{
			AdjustMaxType(pJunqi, iDir2);
		}
	}
	else if( 1==bLandMove && iDir1%2!=ENGINE_DIR%2 )
	{
	    AdjustMaxType(pJunqi, iDir1);
	}

	//assert( pJunqi->Lineup[0][24].type!=DILEI );
	assert( aseertChess(pSrc) );
	assert( aseertChess(pDst) );
}

void InitChess(Junqi* pJunqi, u8 *data)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	int i;

	pJunqi->eTurn = pHead->iDir;
	for(i=0; i<4; i++)
	{
		SetChess(pJunqi,i);
	}
	for(i=0; i<9; i++)
	{
		pJunqi->NineGrid[i].type = NONE;
	}
	memset(pJunqi->aInfo, 0, sizeof(pJunqi->aInfo));
	AdjustMaxType(pJunqi,(ENGINE_DIR+1)%4);
	AdjustMaxType(pJunqi,(ENGINE_DIR+3)%4);
}

void InitBoard(Junqi* pJunqi)
{
	for(int i=0; i<4; i++)
	{
		for(int j=0; j<30; j++)
		{
			CreatGraphVertex(pJunqi,&pJunqi->ChessPos[i][j]);
		}
	}
	InitNineGrid(pJunqi);
	InitBoardGraph(pJunqi);
	InitCurveRail(pJunqi);
}
