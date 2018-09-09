/*
 * junqi.c
 *
 *  Created on: Aug 17, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "path.h"

//>=司令：1，>=军长：2，>=师长：4，>=旅长：6
const u8 aMaxTypeNum[14] =
{
	0,0,1,3,2,1,2,4,6,8,10,13,16,19
};

Junqi *JunqiOpen(void)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));
	pthread_mutex_init(&pJunqi->mutex, NULL);
	return pJunqi;
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
			pJunqi->bStart = 0;
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
			rc = 0;
		}
	}
	return rc;
}

int GetTypeNum(u8 *aBombNum, const u8 *aTpyeNum, int type)
{
	int i;
	int sum = 0;
	int sum1 = 0;
	for(i=SILING; i<=type; i++)
	{
		sum += aTpyeNum[i];
		sum1 += aBombNum[i];
	}

	sum -= (sum1<(2-aTpyeNum[ZHADAN]))?sum1:(2-aTpyeNum[ZHADAN]);
	return sum;
}

void AdjustMaxType(Junqi *pJunqi, int iDir)
{
	int i;
	ChessLineup *pLineup;
	u8 aTypeNum[14] = {0};
	u8 aBombNum[14] = {0};

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->type==NONE || pLineup->type==DARK )
		{
			continue;
		}
		aTypeNum[pLineup->type]++;
		if( pLineup->bBomb )
		{
			aBombNum[pLineup->type]++;
		}
	}
	if( aTypeNum[GONGB]>3 )
	{
		log_b("gongb zhad %d %d",aTypeNum[GONGB], aTypeNum[ZHADAN]);
		aTypeNum[ZHADAN] += aTypeNum[GONGB]-3;
		//assert(0);
	}

	for(i=0; i<30; i++)
	{
		pLineup = &pJunqi->Lineup[iDir][i];
		if( pLineup->type==NONE || pLineup->type==DARK )
		{
			continue;
		}
		while( pLineup->mx_type<pLineup->type )
		{
			if( GetTypeNum(aBombNum,aTypeNum,pLineup->mx_type)<aMaxTypeNum[pLineup->mx_type] )
			{
				break;
			}
			else
			{
				pLineup->mx_type++;
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
		if( pDst->pLineup->type==DARK )
		{
			pDst->pLineup->bBomb = 1;
		}

		if( (pResult->extra_info&0x02) && !(pResult->extra_info&0x04))
		{
			pDst->pLineup->type = ZHADAN;
		}
		else if( pSrc->pLineup->type!=ZHADAN )
		{
			pDst->pLineup->type = pSrc->pLineup->type;
		}

	}
	else
	{
		if( pSrc->pLineup->type==DARK )
		{
			pSrc->pLineup->bBomb = 1;
		}

		if( ((pResult->extra_info&0x04) && !(pResult->extra_info&0x02)) ||
				pDst->pLineup->type==DILEI )
		{
			pSrc->pLineup->type = ZHADAN;
		}
		else if( pDst->pLineup->type!=ZHADAN )
		{
			pSrc->pLineup->type = pDst->pLineup->type;
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

	assert( aseertChess(pSrc) );
	assert( aseertChess(pDst) );

	iDir1 = pSrc->pLineup->iDir;
	if( type!=MOVE )
	{
		iDir2 = pDst->pLineup->iDir;
	}
	if( pResult->extra_info&0x02 )
	{
		p.x = pResult->junqi_src[0];
		p.y = pResult->junqi_src[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->type = JUNQI;
		pSrc->pLineup->type = SILING;
		pJunqi->aInfo[iDir1].bShowFlag = 1;
	}
	if( pResult->extra_info&0x04 )
	{
		p.x = pResult->junqi_dst[0];
		p.y = pResult->junqi_dst[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->type = JUNQI;
		pDst->pLineup->type = SILING;
		pJunqi->aInfo[iDir2].bShowFlag = 1;
	}
	if( pDst->isStronghold && ((pResult->extra_info&1)==0) )
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
		pJunqi->aInfo[pDst->iDir].bShowFlag = 1;
	}

	if( iDir1%2!=ENGINE_DIR%2 && pSrc->pLineup->index>=20 )
	{
		pSrc->pLineup->isNotLand = 1;
	}

	if( pSrc->pLineup->type==DARK )
	{
		if( !IsEnableMove(pJunqi, pSrc, pDst) )
		{
			pSrc->pLineup->type = GONGB;
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

			JudgeIfBomb(pJunqi, pSrc, pDst, pResult);
		}
		else if( pSrc->pLineup->iDir%2!=ENGINE_DIR%2 )
		{
			if( pDst->type!=DILEI && pDst->type!=JUNQI &&
					(pDst->type<=pSrc->pLineup->type || pSrc->pLineup->type==DARK) )
			{
				log_a("change type %d %d",pSrc->pLineup->type,pDst->type);
				pSrc->pLineup->type = pDst->type-1;
			}
			else if( pDst->type==DILEI )
			{
				pSrc->pLineup->type = GONGB;
			}
		}
		else
		{
			if( pSrc->pLineup->type==PAIZH )
			{
				pDst->pLineup->type = GONGB;
			}
		}

		if( pResult->extra_info&1 )
		{
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

		if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
		{
			if( pSrc->type==GONGB )
			{
				pDst->pLineup->isNotLand = 1;
			}
			if( pDst->pLineup->type>=pSrc->type || pDst->pLineup->type==DARK )
			{
				pDst->pLineup->type = pSrc->type-1;
			}
			if( pDst->pLineup->type<pDst->pLineup->mx_type )
			{
				pDst->pLineup->type = DILEI;
			}
		}
		else
		{
			if( pDst->pLineup->type==PAIZH )
			{
				pSrc->pLineup->type = GONGB;
			}
		}

	}
	pSrc->type = NONE;
	//预测敌方最大的棋子
	if( iDir1%2!=ENGINE_DIR%2 )
	{
		AdjustMaxType(pJunqi, iDir1);
	}
	else if( type!=MOVE )
	{
		AdjustMaxType(pJunqi, iDir2);
	}

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
