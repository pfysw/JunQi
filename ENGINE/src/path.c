#include "junqi.h"
#include "event.h"
#include "path.h"

#undef log_a
#define log_a(format,...)

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

	if( pSrc->isStronghold )
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

void ClearPathCnt(Junqi *pJunqi)
{
	int i,j;

	for(i=0; i<17; i++)
	{
		for(j=0; j<17; j++)
		{
			if( pJunqi->aBoard[i][j].pAdjList )
			{
				pJunqi->aBoard[i][j].pAdjList->pChess->pathCnt  = 0;
			}
		}
	}
}

void AddToPath(Engine *pEngine, BoardChess *pSrc, int iPath)
{
	GraphPath *p;
	GraphPath *pHead;

	pHead= pEngine->pPath[iPath];
	p = (GraphPath *)malloc(sizeof(GraphPath));
	memset(p, 0, sizeof(GraphPath));
    p->pChess = pSrc;

	if( pHead==NULL )
	{
		p->isHead = 1;
		p->pNext = p;
		p->pPrev = p;
		pEngine->pPath[iPath] = p;
	}
	else
	{
		//插入尾部
		p->pNext = pHead;
		p->pPrev = pHead->pPrev;
		pHead->pPrev = p;
		p->pPrev->pNext = p;
	}

}

void RemovePathTail(Engine *pEngine, int iPath)
{
	GraphPath *pTail;
	GraphPath *pHead;

	pHead = pEngine->pPath[iPath];
	if( pHead==NULL)
	{
		return;
	}
	pTail = pHead->pPrev;

	if( !pTail->isHead )
	{
		pTail->pPrev->pNext = pHead;
		pHead->pPrev = pTail->pPrev;
		free(pTail);
	}
	else
	{
		free(pTail);
		pEngine->pPath[iPath] = NULL;
	}
}


u8 CanMovetoJunqi(Engine *pEngine, BoardChess *pSrc)
{

	u8 rc = 0;
	Junqi *pJunqi = pEngine->pJunqi;
	GraphPath *pHead;
	BoardChess *pNbr;
	u8 flag = 0;
    int i;
    int x,y;

    if( pSrc->pathCnt>3 )
    {
    	return 0;
    }
    //该点正在当前遍历的路径中
    //该标志位必须在本函数内清0
    //如果后续碰到该点不要遍历
    pSrc->pathFlag = 1;
	if( pEngine->pPath[1]==NULL )
	{
		log_a("index %d %s",pSrc->index,aTypeName[pSrc->type]);
		flag = 1;
		pSrc->pathCnt++;
		AddToPath(pEngine, pSrc, 1);
	}
	pHead = pEngine->pPath[1];
	for(i=0; i<18; i++)
	{
		if( i==4 ) continue;
		if( i<9 )
		{
			x = pSrc->point.x+1-i%3;
			y = pSrc->point.y+i/3-1;
		}
		else if( pSrc->isNineGrid )
		{
			if( i==13 ) continue;
			x = pSrc->point.x+(1-(i-9)%3)*2;
			y = pSrc->point.y+((i-9)/3-1)*2;
		}
		else
		{
			break;
		}
		if( x<0||x>16||y<0||y>16 ) continue;

		if( pJunqi->aBoard[x][y].pAdjList )
		{
			pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;
			if( !IsEnableMove(pJunqi, pSrc, pNbr) )
			{
				continue;
			}
			if( pNbr->isStronghold && pNbr->iDir%2!=ENGINE_DIR%2 &&
				!pJunqi->aInfo[pNbr->iDir].bDead  )
			{
                if( pSrc!=pHead->pChess )
                {
					if(  pSrc->pathCnt+1>=pNbr->pathCnt && pNbr->pathCnt!=0 )
					{
						break;
					}
					pNbr->pathCnt = pSrc->pathCnt+1;
                }
                else
                {
                	pNbr->pathCnt = 1;
                }

				if( pNbr->type==JUNQI )
				{
					rc |= 1;
					SETBIT(aEventBit, JUNQI_EVENT);
				}
				else
				{
					//找到军旗后也不要挖假旗了
					//亮旗后就不要挖假旗了
					//必须要没挖过,必须要比旅长小
					if( (!TESTBIT(aEventBit, JUNQI_EVENT)) && !pJunqi->aInfo[pNbr->iDir].bShowFlag &&
							pNbr->type==DARK && pHead->pChess->type>LVZH )
					{
						rc |= 2;
						SETBIT(aEventBit, MOVE_EVENT);
					}
					else
					{
						break;
					}
				}

				if( pHead->pChess!=pSrc )
				{
					AddToPath(pEngine, pSrc, 1);
				}
				AddToPath(pEngine, pNbr, 1);

				pEngine->pMove[0] = pHead->pChess;
				pEngine->pMove[1] = pHead->pNext->pChess;


				RemovePathTail(pEngine, 1);
				RemovePathTail(pEngine, 1);
				log_a("find path %d %d %d %d %d %d",pNbr->pathCnt,
						pSrc->pathCnt,pSrc->point.x,
						pSrc->point.y,pNbr->point.x,pNbr->point.y);
				break;
			}

			else if( pNbr->pathFlag )
			{
				continue;
			}

			else if( pNbr->isStronghold || pNbr->type!=NONE )
			{
				continue;
			}
			else
			{
				if( !IsEnableMove(pJunqi, pHead->pPrev->pChess, pNbr) )
				{
					log_a("i %d change path %d %d %d %d %d %d",i,pNbr->pathCnt,
							pSrc->pathCnt,pSrc->point.x,
							pSrc->point.y,pNbr->point.x,pNbr->point.y);
					if( pSrc->pathCnt+1>pNbr->pathCnt && pNbr->pathCnt!=0 )
					{
						log_a("i %d %d %d",i,pNbr->pathCnt,pSrc->pathCnt);
						continue;
					}

					if( pSrc->sameFlag )
					{
						continue;
					}
					if( pSrc->pathCnt+1==pNbr->pathCnt )
					{
						pNbr->sameFlag = 1;
					}


					pNbr->pathCnt = pSrc->pathCnt+1;
					AddToPath(pEngine, pSrc, 1);
					rc |= CanMovetoJunqi(pEngine, pNbr);
					RemovePathTail(pEngine, 1);
				}
				else
				{
					log_a("i %d same path %d %d %d %d %d %d",i,pNbr->pathCnt,
							pSrc->pathCnt,pSrc->point.x,
							pSrc->point.y,pNbr->point.x,pNbr->point.y);
					if( pSrc->pathCnt>pNbr->pathCnt && pNbr->pathCnt!=0 )
					{
						log_a("i %d %d %d",i,pNbr->pathCnt,pSrc->pathCnt);
						continue;
					}

					//只有发现更短路径pSrc->pathCnt<pNbr->pathCnt才继续搜索
					//遍历到的点路径长度与之前相同，可能存在新路径继续走直线
					//而原来的路径出现拐弯，从而使pSrc->pathCnt<pNbr->pathCnt
					//否则就不用继续遍历了
					if( pSrc->sameFlag && pSrc->pathCnt==pNbr->pathCnt )
					{
						continue;
					}
					if( pSrc->pathCnt==pNbr->pathCnt )
					{
						pNbr->sameFlag = 1;
					}

					pNbr->pathCnt = pSrc->pathCnt;
					rc |= CanMovetoJunqi(pEngine, pNbr);
				}
				//在相同路径长度的点遍历完毕后清除标志位
				pNbr->sameFlag = 0;
			}
		}

	}

	if( flag==1 )
	{
		RemovePathTail(pEngine, 1);
	}
	pSrc->pathFlag = 0;
	return rc;
}


void SetNewNodeIndex(
        Junqi *pJunqi,
        JunqiPathList *pSrcData,
        int index,
        int isLeft,
        int pathIndex)
{
    JunqiPathList *p;
    p = (JunqiPathList *)malloc(sizeof(JunqiPathList));
    memset(p,0,sizeof(JunqiPathList));
    p->index = index;
    p->iDir = pSrcData->iDir;
    pSrcData->pNext[pathIndex] = p;
    GenJunqiPath(pJunqi, p, isLeft);
}

void GenJunqiPath(
        Junqi *pJunqi,
        JunqiPathList *pSrcData,
        int isLeft)
{
    int iDir = pSrcData->iDir;
    int index = pSrcData->index;
    BoardChess *pSrc;

    pSrc = &pJunqi->ChessPos[iDir][index];
    if( index<20 )
    {
        if( pSrc->isRailway )
        {
            SetNewNodeIndex(pJunqi,pSrcData,index+5,isLeft,0);
            if( index==10 )
            {
                SetNewNodeIndex(pJunqi,pSrcData,16,isLeft,1);
            }
            else if( index==14 )
            {
                SetNewNodeIndex(pJunqi,pSrcData,18,isLeft,1);
            }
        }
        else
        {
            if( isLeft )
            {
                SetNewNodeIndex(pJunqi,pSrcData,23,isLeft,0);
            }
            else
            {
                SetNewNodeIndex(pJunqi,pSrcData,21,isLeft,0);
            }
            SetNewNodeIndex(pJunqi,pSrcData,22,isLeft,1);
        }
    }
    else if( pSrc->isRailway )
    {
        if( isLeft )
        {
            if( index>20 )
            {
                SetNewNodeIndex(pJunqi,pSrcData,index+5,isLeft,1);
                SetNewNodeIndex(pJunqi,pSrcData,index-1,isLeft,0);
            }
        }
        else
        {
            if( index<24 )
            {
                SetNewNodeIndex(pJunqi,pSrcData,index+5,isLeft,1);
                SetNewNodeIndex(pJunqi,pSrcData,index+1,isLeft,0);
            }
        }
    }
    else
    {
        if( index==25 )
        {
            SetNewNodeIndex(pJunqi,pSrcData,26,isLeft,0);
        }
        else if( index==29 )
        {
            SetNewNodeIndex(pJunqi,pSrcData,28,isLeft,0);
        }
        else if( index==27 )
        {
            SetNewNodeIndex(pJunqi,pSrcData,26,isLeft,0);
            SetNewNodeIndex(pJunqi,pSrcData,28,isLeft,1);
        }
    }

}

void InitJunqiPath(Junqi *pJunqi)
{
    int i;
    JunqiPathList *p;
    for(i=0; i<4; i++)
    {
        assert(pJunqi->paPath[i][0]==NULL);
        assert(pJunqi->paPath[i][1]==NULL);
        p = (JunqiPathList*)malloc(sizeof(JunqiPathList));
        memset(p,0,sizeof(JunqiPathList));
        pJunqi->paPath[i][0] = p;
        p->index = 0;
        p->iDir = i;
        GenJunqiPath(pJunqi, p, 0);
        p = (JunqiPathList*)malloc(sizeof(JunqiPathList));
        memset(p,0,sizeof(JunqiPathList));
        pJunqi->paPath[i][1] = p;
        p->index = 4;
        p->iDir = i;
        GenJunqiPath(pJunqi, p, 1);
    }
}

void AddToJunqiPath(
        Engine *pEngine,
        u8 index,
        u8 iPath )
{
    JunqiPath *p;
    JunqiPath *pHead;

    pHead = pEngine->pJunqiPath[iPath];
    p = (JunqiPath *)malloc(sizeof(JunqiPath));
    memset(p, 0, sizeof(JunqiPath));

    p->index = index;
//    p->nChess = pSrcData->nChess;
//    p->nMayLand = pSrcData->nMayLand;


    if( pHead==NULL )
    {
        p->isHead = 1;
        p->pNext = p;
        p->pPre = p;
        pEngine->pJunqiPath[iPath] = p;
    }
    else
    {
        //插入尾部
        p->pNext = pHead;
        p->pPre = pHead->pPre;
        pHead->pPre = p;
        p->pPre->pNext = p;
    }


}

void ClearJunqiPath(Engine *pEngine, int iPath)
{
    JunqiPath *p;
    JunqiPath *pTmp;

    if( pEngine->pJunqiPath[iPath]==NULL )
    {
        return;
    }
    assert( pEngine->pJunqiPath[iPath]->isHead );

    p = pEngine->pJunqiPath[iPath]->pNext;
    while(1)
    {
        if(p->isHead)
        {
            free(p);
            break;
        }
        else
        {
            pTmp = p;
            p = p->pNext;
            free(pTmp);
        }
    }

    pEngine->pJunqiPath[iPath] = NULL;
}

void RemoveJunqiPath(Engine *pEngine, int iPath)
{
    JunqiPath *pTail;
    JunqiPath *pHead;

    pHead = pEngine->pJunqiPath[iPath];
    if( pHead==NULL)
    {
        return;
    }
    pTail = pHead->pPre;

    if( !pTail->isHead )
    {
        pTail->pPre->pNext = pHead;
        pHead->pPre = pTail->pPre;
        free(pTail);
    }
    else
    {
        free(pTail);
        pEngine->pJunqiPath[iPath] = NULL;
    }

}

void UpdateJunqiPath(Engine *pEngine,int iPath1, int iPath2)
{
    JunqiPath *p;

    assert( pEngine->pJunqiPath[iPath1]==NULL );
    assert( pEngine->pJunqiPath[iPath2]!=NULL );

    p = pEngine->pJunqiPath[iPath2];
    assert( p->isHead );

    do
    {
        AddToJunqiPath(pEngine, p->index, iPath1);

        if( p->pNext->isHead )
        {
            break;
        }

        p = p->pNext;
    }while(1);
}

void SetPathData(Engine *pEngine,JunqiPath *pNode,int iDir)
{
    ClearJunqiPath(pEngine, 0);
    UpdateJunqiPath(pEngine, 0, 1);
    pEngine->pJunqiPath[0]->nChess = pNode->nChess;
    pEngine->pJunqiPath[0]->nMayLand = pNode->nMayLand;
    pEngine->pJunqiPath[0]->iDir = iDir;
}

void UpdatePathData(
        Engine *pEngine,
        JunqiPath *pNode,
        JunqiPath *pHead,
        int iDir)
{
    if( NULL==pHead )
    {
        SetPathData(pEngine,pNode,iDir);
    }
    else
    {
        if( pNode->nMayLand<pHead->nMayLand )
        {
            SetPathData(pEngine,pNode,iDir);
        }
        else if( pNode->nMayLand==pHead->nMayLand )
        {
            if( pNode->nChess<pHead->nChess )
            {
                SetPathData(pEngine,pNode,iDir);
            }
        }
    }
}

void GetJunqiPath(
        Engine *pEngine,
        JunqiPathList *pSrcData )
{
    Junqi *pJunqi = pEngine->pJunqi;
    JunqiPath *pHead = pEngine->pJunqiPath[0];
    JunqiPath *pNode;
    JunqiPath *pPre;
    JunqiPathList *pNext;
    BoardChess *pSrc;
    int iDir = pSrcData->iDir;
    int index = pSrcData->index;
    int i;

    assert( pSrcData!=NULL );
    pSrc = &pJunqi->ChessPos[iDir][index];

    //有棋子在营里，此路不通
    if( pSrc->isCamp && pSrc->type!=NONE &&
            (pSrc->pLineup->iDir&1)==(pSrc->iDir&1) )
    {
        return;
    }

    AddToJunqiPath(pEngine, pSrcData->index, 1);

    pNode = pEngine->pJunqiPath[1]->pPre;
    pPre = pNode->pPre;
    pNode->nChess = pPre->nChess;
    pNode->nMayLand = pPre->nMayLand;

    if( !pSrc->isStronghold )
    {
        if( pSrc->type!=NONE )
        {

            if( pSrc->pLineup->isNotLand )
            {
                pNode->nChess++;
            }
            else
            {
                pNode->nMayLand++;
            }
            if( (pSrc->pLineup->iDir&1)!=(pSrc->iDir&1) )
            {
                pNode->nChess = 0;
                pNode->nMayLand = 0;
            }
        }

        for(i=0; i<2; i++)
        {
            pNext = pSrcData->pNext[i];
            if( NULL==pNext ) continue;
            pNext->nChess = pSrcData->nChess;
            pNext->nMayLand = pSrcData->nMayLand;
            GetJunqiPath(pEngine, pNext);
        }
    }
    else
    {
        if( pJunqi->aInfo[iDir].bShowFlag )
        {
            if( pSrc->type==JUNQI )
            {
                UpdatePathData(pEngine,pNode,pHead,iDir);
            }
        }
        else
        {
            if( pSrc->iDir%2!=ENGINE_DIR || pSrc->type==JUNQI )
            {
                UpdatePathData(pEngine,pNode,pHead,iDir);
            }
        }
    }
    RemoveJunqiPath(pEngine,1);
}

int EvaluatePathValue(Junqi *pJunqi, u8 nChess, u8 nMayLand)
{
    int val = 0;
    Engine *pEngine = pJunqi->pEngine;
    Value_Parameter *pVal;

    pVal= &pEngine->valPara;
    if( nMayLand==0 )
    {
        switch(nChess)
        {
        case 0:
            val = 0;
            break;
        case 1:
            val = pVal->vPathChess<<1;
            break;
        default:
            val = (pVal->vPathChess<<1)+pVal->vPathChess;
            break;
        }
    }
    else if( nMayLand==1 )
    {
        switch(nChess)
        {
        case 0:
            val = pVal->vPathLand<<1;
            break;
        case 1:
            val = (pVal->vPathLand<<1)+pVal->vPathChess;
            break;
        default:
            val = (pVal->vPathChess<<1)+(pVal->vPathLand<<1);
            break;
        }
    }
    else
    {
        switch(nChess)
        {
        case 0:
            val = pVal->vPathLand<<2;
            break;
        default:
            val = (pVal->vPathLand<<2)+pVal->vPathChess;
            break;
        }
    }

    return val;
}
int CalJunqiPathValue(Junqi *pJunqi, int iDir)
{
    int value = 0;
    Engine *pEngine = pJunqi->pEngine;
    JunqiPath *pPath;
    u16 val1,val2;

    ClearJunqiPath(pEngine, 0);

    GetJunqiPath(pEngine,pJunqi->paPath[iDir][0]);
    pPath = pEngine->pJunqiPath[0];
    val1 = EvaluatePathValue(pJunqi,pPath->nChess,pPath->nMayLand);

    log_a("nChess %d nLand %d",pPath->nChess,pPath->nMayLand);
    ClearJunqiPath(pEngine, 0);
    GetJunqiPath(pEngine,pJunqi->paPath[iDir][1]);
    pPath = pEngine->pJunqiPath[0];
    val2 = EvaluatePathValue(pJunqi,pPath->nChess,pPath->nMayLand);
    log_a("nChess %d nLand %d",pPath->nChess,pPath->nMayLand);

//    if( pJunqi->aInfo[iDir].bShowFlag )
//    {
//        if( pJunqi->Lineup[iDir][26].type==JUNQI )
//        {
//            val1 = val1<<1;
//            val2 = val2>>1;
//        }
//        else
//        {
//            val1 = val1>>1;
//            val2 = val2<<1;
//        }
//    }
    value = val1+val2;
    log_a("val1 %d val2 %d",val1,val2);
    log_a("dir %d val %d",iDir,value);


    return value;
}

int GetJunqiPathValue(Junqi *pJunqi, int iDir)
{
    int value = 0;
    int i;

    for(i=0; i<4; i++)
    {
        if( !pJunqi->aInfo[i].bDead )
        {
            if( i%2==ENGINE_DIR )
            {
                value += CalJunqiPathValue(pJunqi,i);
            }
            else
            {
                value -= CalJunqiPathValue(pJunqi,i);
            }
        }
    }

    if( iDir%2!=ENGINE_DIR )
    {
        value = -value;
    }


    return value;
}


