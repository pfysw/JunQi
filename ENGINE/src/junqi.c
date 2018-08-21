/*
 * junqi.c
 *
 *  Created on: Aug 17, 2018
 *      Author: Administrator
 */
#include "junqi.h"

Junqi *JunqiOpen(void)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));

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
			else if(!isInit)
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
		pJunqi->Lineup[dir][i].iDir = dir;
		pJunqi->Lineup[dir][i].pChess = &pJunqi->ChessPos[dir][i];
		pJunqi->Lineup[dir][i].bDead = 0;
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

u8 IsSameRail(
		Junqi *pJunqi,
		BoardGraph *pSrc,
		BoardGraph *pDst,
		enum RailType type)
{
	BoardChess *pSrcChess;
	BoardChess *pDstChess;
	u8 rc =0;

	pSrcChess = pSrc->pAdjList->pChess;
	pDstChess = pDst->pAdjList->pChess;

	switch(type)
	{
	case HORIZONTAL_RAIL:
		if( pSrcChess->point.x==pDstChess->point.x )
			rc = 1;
		break;
	case VERTICAL_RAIL:
		if( pSrcChess->point.y==pDstChess->point.y )
			rc = 1;
		break;
	case CURVE_RAIL:
		assert( pSrcChess->eCurveRail>0 );
		if( pSrcChess->eCurveRail==pDstChess->eCurveRail  )//&& pSrcChess->eCurveRail>0
			rc = 1;
		break;
	default:
		break;
	}

	return rc;
}

u8 GetRailPath(
		Junqi *pJunqi,
		BoardGraph *pSrc,
		BoardGraph *pDst,
		enum RailType type)
{
	AdjNode *p;
	BoardGraph *pVertex;
	u8 rc = 0;

	pSrc->passCnt++;

	for(p=pSrc->pAdjList->pNext; p!=NULL; p=p->pNext)
	{
		pVertex = &pJunqi->aBoard[p->pChess->point.x][p->pChess->point.y];
		if( p->pChess==pDst->pAdjList->pChess )
		{
			return 1;
		}
		else if( p->pChess->type!=NONE )
		{
			continue;
		}
		else if( pVertex->passCnt!=0 )
		{
			continue;
		}
		else if( type!=GONGB_RAIL && !IsSameRail(pJunqi, pSrc, pVertex, type))
		{
            continue;
		}
		else
		{
			rc = GetRailPath(pJunqi, pVertex, pDst, type);
			if( rc )  break;
		}
	}

	return rc;
}

void ClearPassCnt(Junqi *pJunqi)
{
	int i,j;

	for(i=0; i<17; i++)
	{
		for(j=0; j<17; j++)
		{
			pJunqi->aBoard[i][j].passCnt = 0;
		}
	}
}

int IsEnableMove(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst)
{
	int rc = 0;

	BoardGraph *pVertex1;
	BoardGraph *pVertex2;

	ClearPassCnt(pJunqi);

	if(pSrc->isStronghold)
	{
		return rc;
	}
	else if( pDst->isCamp && pDst->type!=NONE )
	{
		return rc;
	}
	else if( pSrc->type==DILEI || pSrc->type==JUNQI )
	{
		return rc;
	}


	//如果是相邻的格子（包括斜相邻）
	if( ((pDst->point.x-pSrc->point.x)>=-1 && (pDst->point.x-pSrc->point.x)<=1) &&
			((pDst->point.y-pSrc->point.y)>=-1 && (pDst->point.y-pSrc->point.y)<=1) )
	{
		//营与周围的格子都是相邻的
		if( pSrc->isCamp || pDst->isCamp )
		{
			rc = 1;
		}
		//非斜相邻
		else if( pDst->point.x==pSrc->point.x || pDst->point.y==pSrc->point.y)
		{
			rc = 1;
		}
	}
	if( !rc && pSrc->isRailway && pDst->isRailway )
	{
		pVertex1 = &pJunqi->aBoard[pSrc->point.x][pSrc->point.y];
		pVertex2 = &pJunqi->aBoard[pDst->point.x][pDst->point.y];

		if( pSrc->type==GONGB )
		{
			rc = GetRailPath(pJunqi, pVertex1, pVertex2, GONGB_RAIL);
		}
		else if( pDst->point.x==pSrc->point.x )
		{
			rc = GetRailPath(pJunqi, pVertex1, pVertex2, HORIZONTAL_RAIL);
		}
		else if( pDst->point.y==pSrc->point.y )
		{
			rc = GetRailPath(pJunqi, pVertex1, pVertex2, VERTICAL_RAIL);
		}
		else if( pDst->eCurveRail>0 && pDst->eCurveRail==pSrc->eCurveRail )
		{
			rc = GetRailPath(pJunqi, pVertex1, pVertex2, CURVE_RAIL);
		}
	}

	return rc;
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

	assert( aseertChess(pSrc) );
	assert( aseertChess(pDst) );
	if( type==EAT || type==BOMB )
	{
		pDst->pLineup->bDead = 1;

		assert(pDst->type!=NONE);
		if( type==BOMB )
		{
			pDst->type = NONE;
			pSrc->pLineup->bDead = 1;

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
	}

	if( pResult->extra_info&0x02 )
	{
		p.x = pResult->junqi_src[0];
		p.y = pResult->junqi_src[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->type = JUNQI;
	}
	if( pResult->extra_info&0x04 )
	{
		p.x = pResult->junqi_dst[0];
		p.y = pResult->junqi_dst[1];
		pJunqiChess = pJunqi->aBoard[p.x][p.y].pAdjList->pChess;
		pJunqiChess->pLineup->type = JUNQI;
		pJunqiChess->type = JUNQI;
	}

	pSrc->type = NONE;
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
