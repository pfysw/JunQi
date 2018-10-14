/*
 * search.c
 *
 *  Created on: Sep 20, 2018
 *      Author: Administrator
 */
#include "engine.h"
#include "junqi.h"
#include "movegen.h"
#include "path.h"
#include "search.h"
#include "evaluate.h"
#include <time.h>
#include <windows.h>

//#include <omp.h>


#define OffSET(type,field) ((size_t)&(((type*)0)->field))
#define VAR_OFFSET 8

//int tt = 0;
void PushMoveToStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove )
{

	Engine *pEngine = pJunqi->pEngine;
	PositionList *p;
	PositionList *pHead;
	PositionData *pStorage;
	int iDir;

	p = (PositionList *)malloc(sizeof(PositionList));
	memset(p, 0, sizeof(PositionList));//初始化
	pHead = pEngine->pPos;
	pStorage = &p->data;

	if( pHead==NULL )
	{

		p->isHead = 1;
		p->pNext = p;
		p->pPre = p;
		pEngine->pPos = p;

	}
	else
	{

		p->pNext = pHead;
		p->pPre = pHead->pPre;
		pHead->pPre->pNext = p;
		pHead->pPre = p;
	}

	memcpy(&pStorage->xSrcChess, pSrc, VAR_OFFSET);
	memcpy(&pStorage->xDstChess, pDst, VAR_OFFSET);


	memcpy(&pStorage->xSrcLineup, pSrc->pLineup, sizeof(ChessLineup));

	//todo 目前没有考虑投降、跳过、无棋可走的情况
	if( pMove->result!=MOVE )
	{
		memcpy(&pStorage->xDstLineup, pDst->pLineup, sizeof(ChessLineup));
		if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
		{
			iDir = pDst->pLineup->iDir;
		}
		else
		{
			iDir = pSrc->pLineup->iDir;
		}

		pStorage->junqi_chess_type[0] = pJunqi->ChessPos[iDir][26].type;
		pStorage->junqi_chess_type[1] = pJunqi->ChessPos[iDir][28].type;
		pStorage->junqi_type[0] = pJunqi->Lineup[iDir][26].type;
		pStorage->junqi_type[1] = pJunqi->Lineup[iDir][28].type;
		memcpy(&pStorage->info[0], &pJunqi->aInfo[pSrc->pLineup->iDir], sizeof(PartyInfo));
		memcpy(&pStorage->info[1], &pJunqi->aInfo[pDst->pLineup->iDir], sizeof(PartyInfo));
        for(int i=0; i<30; i++)
        {
        	pStorage->mx_type[i] = pJunqi->Lineup[iDir][i].mx_type;
        }

	}

}

void PopMoveFromStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove )
{
	Engine *pEngine = pJunqi->pEngine;
	PositionList *pHead;
	PositionData *pStorage;
	PositionList *pTail;

	int iDir,iDir1;

	pHead = pEngine->pPos;
	pTail = pHead->pPre;
	pStorage = &pTail->data;

	assert( pHead!=NULL );

	memcpy(pSrc, &pStorage->xSrcChess, VAR_OFFSET);
	memcpy(pDst, &pStorage->xDstChess, VAR_OFFSET);
	memcpy(pSrc->pLineup, &pStorage->xSrcLineup, sizeof(ChessLineup));


	if( pMove->result!=MOVE )
	{
		memcpy(pDst->pLineup, &pStorage->xDstLineup, sizeof(ChessLineup));

		if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
		{
			iDir = pDst->pLineup->iDir;
		}
		else
		{
			iDir = pSrc->pLineup->iDir;
		}

		pJunqi->Lineup[iDir][26].type = pStorage->junqi_type[0];
		pJunqi->Lineup[iDir][28].type = pStorage->junqi_type[1];
		pJunqi->ChessPos[iDir][26].type = pStorage->junqi_chess_type[0];
		pJunqi->ChessPos[iDir][28].type = pStorage->junqi_chess_type[1];


        for(int i=0; i<30; i++)
        {
        	pJunqi->Lineup[iDir][i].mx_type = pStorage->mx_type[i];
        }

    	//暂时不考虑无棋可走的情况
    	iDir1 = pDst->pLineup->iDir;
    	if( pJunqi->aInfo[iDir1].bDead )
    	{
    		ChessLineup *pLineup;
    		for(int i=0; i<30; i++)
    		{
    			pLineup = &pJunqi->Lineup[iDir1][i];
    			if( !pLineup->bDead && pLineup->type!=NONE )
    			{
    				pLineup->pChess->type = pLineup->type;
    			}
    		}
    		pJunqi->aInfo[iDir1].bDead = 0;
    	}
    	//pJunqi->aInfo不要在之前还原，因为要用到判断是否阵亡
    	memcpy(&pJunqi->aInfo[pSrc->pLineup->iDir], &pStorage->info[0], sizeof(PartyInfo));
    	memcpy(&pJunqi->aInfo[pDst->pLineup->iDir], &pStorage->info[1], sizeof(PartyInfo));

	}

	if( !pTail->isHead )
	{
		pTail->pPre->pNext = pHead;
		pHead->pPre = pTail->pPre;
		free(pTail);
	}
	else
	{
		free(pTail);
		pEngine->pPos = NULL;
	}
}

void MakeNextMove(Junqi *pJunqi, MoveResultData *pResult)
{
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;

	p1.x = pResult->src[0]%17;
	p1.y = pResult->src[1]%17;
	p2.x = pResult->dst[0]%17;
	p2.y = pResult->dst[1]%17;

	pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;

	assert( pSrc->type!=NONE );
	pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;
	PushMoveToStack(pJunqi, pSrc, pDst, pResult);
	PlayResult(pJunqi, pSrc, pDst, pResult);
	ChessTurn(pJunqi);



}

void UnMakeMove(Junqi *pJunqi, MoveResultData *pResult)
{
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;


	p1.x = pResult->src[0]%17;
	p1.y = pResult->src[1]%17;
	p2.x = pResult->dst[0]%17;
	p2.y = pResult->dst[1]%17;
	pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;
	pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;

	PopMoveFromStack(pJunqi, pSrc, pDst, pResult);

}

void SetBestMove(Junqi *pJunqi, MoveResultData *pResult)
{
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;
	Engine *pEngine;

	pEngine = pJunqi->pEngine;
	if( pResult!=NULL )
	{
		p1.x = pResult->src[0]%17;
		p1.y = pResult->src[1]%17;
		p2.x = pResult->dst[0]%17;
		p2.y = pResult->dst[1]%17;
		pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;
		pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;
		pEngine->pBest[0] = pSrc;
		pEngine->pBest[1] = pDst;
	}

}

int TimeOut(Junqi *pJunqi)
{
	int rc = 0;
	int search_time;
	search_time = time(NULL)-pJunqi->begin_time;
	if( pJunqi->bMove || pJunqi->bGo || search_time>20 )
	{
		rc = 1;
	}
	//rc = 0;//测试设断点用，否则停下来就超时了
	return rc;
}

int CallAlphaBeta(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir )
{
    int val;

    if( iDir%2==pJunqi->eTurn%2 )
    {
        //下家阵亡轮到对家走
        val = AlphaBeta(pJunqi,depth,alpha,beta);
    }
    else
    {
        val = -AlphaBeta(pJunqi,depth,-beta,-alpha);
    }

    return val;
}

int CheckMoveHash(MoveHash ***paHash, int iKey)
{

    int nHash = 1024;
    int size;
    int h;
    int rc = 0;
    MoveHash *pNew;
    MoveHash *p;



    if( *paHash==NULL )
    {
        size = sizeof(MoveHash *)*nHash;
        *paHash = (MoveHash **)malloc(size+1);
        memset(*paHash,0,size);
    }
    h = iKey&1023;
    if( (*paHash)[h]==NULL )
    {
        size =sizeof(MoveHash);
        pNew = (MoveHash *)malloc(size);
        memset(pNew,0,size);
        pNew->iKey = iKey;
        (*paHash)[h] = pNew;
        pNew = (MoveHash *)malloc(size);
        memset(pNew,0,size);
        pNew->iKey = iKey;
        (*paHash)[1024] = pNew;
    }
    else
    {
        for(p=(*paHash)[h]; p!=NULL; p=p->pNext)
        {
            if( p->iKey==iKey )
            {
                rc = 1;
                break;
            }
        }
        if( !rc )
        {
            size =sizeof(MoveHash);
            pNew = (MoveHash *)malloc(size);
            memset(pNew,0,size);
            pNew->iKey = iKey;
            pNew->pNext = (*paHash)[h];

            (*paHash)[h] = pNew;
            pNew = (MoveHash *)malloc(size);
            memset(pNew,0,size);
            pNew->iKey = iKey;
            pNew->pNext = (*paHash)[1024];
            (*paHash)[1024] = pNew;
        }
    }

    return rc;
}

void ClearMoveHash(MoveHash ***paHash)
{

    MoveHash *p;
    MoveHash *pDel;
    MoveHash *pTemp;
    int h;


    if( *paHash==NULL )
    {
        return;
    }
    for(p=(*paHash)[1024];p!=NULL;)
    {
        h = p->iKey&1023;

        for(pDel=(*paHash)[h];pDel!=NULL;)
        {
            pTemp = pDel;
            pDel = pDel->pNext;
            free(pTemp);
        }
        (*paHash)[h] = NULL;
        pTemp = p;
        p=p->pNext;
        free(pTemp);
    }
}

int GetMoveHash(Junqi* pJunqi, int iDir, int iKey)
{
    int i;
    BoardChess *pSrc;
    ChessLineup *pLineup;
    pJunqi->searche_num[1]++;
    pJunqi->iKey = iKey;
    for(i=0;  i<30; i++)
    {
        pLineup = &pJunqi->Lineup[iDir][i];
        if( pLineup->bDead || pLineup->type==NONE )
        {
            continue;
        }
        pSrc = pLineup->pChess;
        SearchMovePath(pJunqi,pSrc,1);
    }
    return pJunqi->iKey;
}

int GetHashKey(Junqi* pJunqi)
{
    int iKey = 0;
    int iDir;

    iDir = (pJunqi->eTurn)&3;
    if( !pJunqi->aInfo[iDir].bDead )
    {
        iKey = GetMoveHash(pJunqi,iDir,iKey);
    }
    iDir = (pJunqi->eTurn+2)&3;
    if( !pJunqi->aInfo[iDir].bDead )
    {
        iKey = GetMoveHash(pJunqi,iDir,iKey);
    }
    return iKey;
}

int AlphaBeta(
		Junqi *pJunqi,
		int depth,
		int alpha,
		int beta)
{
	MoveList *pHead;
	MoveList *p;
	MoveResultData *pData;
	MoveResultData *pBest = NULL;
	int val;
	int sum = 0;
	static int cnt = 0;
	int iDir = pJunqi->eTurn;
	MoveHash **paHash = NULL;
	int iKey;


	cnt++;
	//遍历到最后一层时计算局面分值
	if( depth==0 )
	{
		val = EvalSituation(pJunqi);
		pJunqi->test_num++;
		//val = 5;
		//EvalSituation是针对引擎评价的，所以对方的分值应取负值
		if( iDir%2!=ENGINE_DIR%2 )
		{
			val = -val;
		}
		cnt--;

		return val;
	}

	//生成着法列表
	pHead = GenerateMoveList(pJunqi, iDir);

	if( pHead!=NULL )
	{
		pBest = &pHead->move;
	}
	//无棋可走时直接跳到下一层
	else
	{
		pJunqi->eTurn = iDir;
		ChessTurn(pJunqi);

		val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir);
	}

    //遍历每一个着法
    for(p=pHead; pHead!=NULL; p=p->pNext)
    {
    	pJunqi->eTurn = iDir;


    	//模拟着法产生后的局面
    	MakeNextMove(pJunqi,&p->move);
    	assert(pJunqi->pEngine->pPos!=NULL);

        LARGE_INTEGER nBeginTime;
        LARGE_INTEGER nEndTime;
        QueryPerformanceCounter(&nBeginTime);
    	iKey = GetHashKey(pJunqi);

        QueryPerformanceCounter(&nEndTime);
        pJunqi->test_time[0] = nEndTime.QuadPart-nBeginTime.QuadPart;
        pJunqi->test_time[1] += pJunqi->test_time[0];
    	//if(iKey==117440526)
    	//log_a("key %d",iKey);
    	if( CheckMoveHash(&paHash,iKey) &&
    	        IsNotSameMove(p) )
    	{
    	    if( p->move.result>MOVE )
    	    {
    	        while( !memcmp( &p->move, &p->pNext->move, 4) )
    	        {
    	            if( p->pNext->isHead ) break;
    	            p = p->pNext;
    	        }
    	    }
    	    UnMakeMove(pJunqi,&p->move);
    	    goto continue_search;
    	}
    	else
    	{
            val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir);
//            log_a("cnt %d val %d per %d",cnt,val,p->percent);
//            SafeMemout((u8*)&p->move, sizeof(p->move));
            //把局面撤回到上一步

            assert(pJunqi->pEngine->pPos!=NULL);
            UnMakeMove(pJunqi,&p->move);
    	}

    	//if( p->move.result!=MOVE && !p->pNext->isHead )
    	if( p->move.result!=MOVE )
    	{
    		sum += val*p->percent;

    		pData = &p->pNext->move;
            //下一个着法
    		if( memcmp(&p->move, pData, 4) || p->pNext->isHead  )
    		{
    			val = sum>>8;
    			sum = 0;
    		}
    		else
    		{
    		    goto continue_search;
    		}
    	}
        //产生截断
    	if( val>=beta )
    	{
    		alpha = val;
    		break;
    	}
    	//更新alpha值
    	if( val>alpha )
    	{
    		pBest = &p->move;
    		alpha = val;
    	}
continue_search:

    	if( p->pNext->isHead )
    	{
    		break;
    	}
        //时间结束或收到go指令结束搜索
    	if( TimeOut(pJunqi) )
    	{
    		break;
    	}

//    	if(cnt==1)
//    	log_c("src %d %d",p->move.src[0],p->move.src[1]);
    }
    cnt--;
    if( 0==cnt )
    {
    	cnt = 0;
    	SetBestMove(pJunqi,pBest);

    }
	ClearMoveList(pHead);
	ClearMoveHash(&paHash);

	return alpha;
}


u8 SendBestMove(Engine *pEngine)
{
	Junqi *pJunqi = pEngine->pJunqi;
	u8 rc = 0;

	if( pEngine->pBest[0]!=NULL && pEngine->pBest[1]!=NULL )
	{
		assert( IsEnableMove(pJunqi, pEngine->pBest[0], pEngine->pBest[1]) );
		SendMove(pJunqi, pEngine->pBest[0], pEngine->pBest[1]);
		pEngine->pBest[0] = NULL;
		pEngine->pBest[1] = NULL;
		rc = 1;
	}


	return rc;
}

void AddMoveToHash(
        Junqi *pJunqi,
        BoardChess *pSrc,
        BoardChess *pDst )
{
    u8 val[4];
//    val[0] = pSrc->point.x;
//    val[1] = pSrc->point.y;
//    val[2] = pDst->point.x;
//    val[3] = pDst->point.y;
    val[0] = pSrc->pLineup->index<<pSrc->iDir;
    val[1] = pSrc->pLineup->index;
    val[2] = pDst->pLineup->index<<pDst->iDir;
    val[3] = pDst->pLineup->index;
    pJunqi->iKey ^= *((int*)val);
    //log_a("val key %08x %08x",*((int*)val),pJunqi->iKey);
}


