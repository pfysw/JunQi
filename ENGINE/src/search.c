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

int free_cnt = 0;
int malloc_cnt = 0;

static LARGE_INTEGER nBeginTime;
static LARGE_INTEGER nEndTime;

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


	//p = (PositionList *)malloc(sizeof(PositionList));
	p = (PositionList *)memsys5Malloc(pJunqi,sizeof(PositionList));

	memset(p, 0, sizeof(PositionList));//初始化
	pHead = pEngine->pPos;
	assert(p!=pHead);
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

	pStorage->eatInList = pEngine->eatInList;
	pStorage->eatIndex = pEngine->eatIndex;
	memcpy(&pStorage->xSrcChess, pSrc, sizeof(BoardChess));
	memcpy(&pStorage->xDstChess, pDst, sizeof(BoardChess));


	memcpy(&pStorage->xSrcLineup, pSrc->pLineup, sizeof(ChessLineup));
	pStorage->xExtraInfo.adjustFlag = 0;
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
		if( pMove->extra_info!=0 || pDst->isStronghold )
		{
		    memcpy(&pStorage->xJunqiLineup[0], &pJunqi->Lineup[iDir][26], sizeof(ChessLineup));
		    memcpy(&pStorage->xJunqiLineup[1], &pJunqi->Lineup[iDir][28], sizeof(ChessLineup));
		}

		memcpy(&pStorage->info[0], &pJunqi->aInfo[pSrc->pLineup->iDir], sizeof(PartyInfo));
		memcpy(&pStorage->info[1], &pJunqi->aInfo[pDst->pLineup->iDir], sizeof(PartyInfo));
        for(int i=0; i<30; i++)
        {
        	pStorage->mx_type[i] = pJunqi->Lineup[iDir][i].mx_type;
        }

	}
	else if( pDst->isStronghold )
	{
	    //进入大本营当死子处理，type数量被更新，所以要入栈
        memcpy(&pStorage->info[0], &pJunqi->aInfo[pSrc->pLineup->iDir], sizeof(PartyInfo));
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

	//一定要放前面，这里可能和pDst重合，放在后面不能恢复到初始状态
    if( pStorage->xExtraInfo.adjustFlag )
    {
        int iDir,index,type;
        iDir = pStorage->xExtraInfo.xLineup.iDir;
        index = pStorage->xExtraInfo.xLineup.index;
        type = pStorage->xExtraInfo.xLineup.type;

        memcpy( &pJunqi->Lineup[iDir][index],
                 &pStorage->xExtraInfo.xLineup, sizeof(ChessLineup) );
        if( !pJunqi->Lineup[iDir][index].bDead )
        {
            pJunqi->Lineup[iDir][index].pChess->type = type;
        }
    }
    pEngine->eatInList = pStorage->eatInList;
    pEngine->eatIndex = pStorage->eatIndex;
	memcpy(pSrc, &pStorage->xSrcChess, sizeof(BoardChess));
	memcpy(pDst, &pStorage->xDstChess, sizeof(BoardChess));
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

        if( pMove->extra_info!=0 || pDst->isStronghold )
        {
            memcpy(&pJunqi->Lineup[iDir][26], &pStorage->xJunqiLineup[0], sizeof(ChessLineup));
            memcpy(&pJunqi->Lineup[iDir][28], &pStorage->xJunqiLineup[1], sizeof(ChessLineup));
        }
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
    else if( pDst->isStronghold )
    {
        memcpy(&pJunqi->aInfo[pSrc->pLineup->iDir], &pStorage->info[0], sizeof(PartyInfo));
    }


	if( !pTail->isHead )
	{
		pTail->pPre->pNext = pHead;
		pHead->pPre = pTail->pPre;
		memsys5Free(pJunqi,pTail);
	}
	else
	{
	    memsys5Free(pJunqi,pTail);
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

	if( pSrc->type==NONE )
	{
	    log_c("debug");
	    sleep(1);
	}

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
	u8 aZero[4] = {0};

	pEngine = pJunqi->pEngine;
	//if( pResult!=NULL )
	if( memcmp(pResult,aZero,4) )
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
    else
    {
        pEngine->pBest[0] = NULL;
        pEngine->pBest[1] = NULL;
    }


}

#define SEARCH_TIME 10
int TimeOut(Junqi *pJunqi)
{
	int rc = 0;
	int search_time;
	static int pre_time = 0;
	search_time = time(NULL)-pJunqi->begin_time;


	if( pJunqi->pJunqiBase==NULL )
	{
        if( pJunqi->bMove || pJunqi->bGo || search_time>SEARCH_TIME )
        {
            rc = 1;
        }
	}
	else
	{
        if( pJunqi->pJunqiBase->bMove || pJunqi->pJunqiBase->bGo || search_time>SEARCH_TIME )
        {
            rc = 1;
        }
	}

//	if(pre_time!=search_time)
//	{
//	    log_a("time %d",search_time);
//	    pre_time = search_time;
//	}


	//rc = 0;//测试设断点用，否则停下来就超时了
	return rc;
}

int CallAlphaBeta(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir,
        u8 isMove)
{
    int val;

    if( iDir%2==pJunqi->eTurn%2 )
    {
        //下家阵亡轮到对家走
        if( isMove )
            val = AlphaBeta(pJunqi,depth,alpha,beta);
        else
            val = AlphaBeta(pJunqi,depth,alpha,INFINITY);
    }
    else
    {
        if( isMove )
            val = -AlphaBeta(pJunqi,depth,-beta,-alpha);
        else
            val = -AlphaBeta(pJunqi,depth,-beta,INFINITY);
    }

    return val;
}

int GetEatHashValue(
        Junqi *pJunqi,
        BoardChess *pDst )
{
    u8 val[4] = {0};

    val[0] = pDst->point.x;
    val[1] = pDst->point.y;

    val[3] = 1;

    return *((int*)val);
    //log_a("val key %08x %08x",*((int*)val),pJunqi->iKey);
}

int AddMoveToHash(
        Junqi *pJunqi,
        MoveList *pMove,
        u8 isEat )
{
    u8 val[4] = {0};

    val[0] = pMove->move.dst[0];
    val[1] = pMove->move.dst[1];
    if( isEat )
    {
        val[3] = 1;
    }
    else
    {
        val[2] = pJunqi->cnt;
    }


    return *((int*)val);
    //log_a("val key %08x %08x",*((int*)val),pJunqi->iKey);
}



int CheckMoveHash(
        Junqi *pJunqi,
        BoardChess *pDst )
{
    int h;
    int rc = 0;
    MoveHash *p;
    int iKey;
    MoveHash ***paHash;
    u8 nCnt;

    paHash = &pJunqi->paHash;
    if( *paHash==NULL )
    {
        return 0;
    }
    if( pJunqi->eSearchType==SEARCH_DEFAULT ||
           ( pJunqi->eSearchType==SEARCH_DEEP &&
              pJunqi->eDeepType==SEARCH_DEFAULT ))
    {
        nCnt = 4;
    }
    else
    {
        nCnt = 2;
    }
    iKey = GetEatHashValue(pJunqi,pDst);
    h = iKey&1023;
    if( (*paHash)[h]==NULL )
    {
        return 0;
    }
    else
    {
        for(p=(*paHash)[h]; p!=NULL; p=p->pNext)
        {
            if( p->iKey==iKey )
            {
                assert( p->eatFlag );
                //每一步搜索完后，立即释放hash表里的内容
                //所以hash表里不会出现历史着法，可以不要这个条件
                //if( (p->iDir&1)!=(pDst->pLineup->iDir&1) )
                if( pJunqi->cnt < p->cnt+nCnt )
                {
                    rc = 1;
                    break;
                }
            }
        }
    }

    return rc;
}

int GetHashMoveValue(MoveList *pMove, int alpha, u8 bSet)
{
    MoveList *p;
    MoveList *pKill = NULL;
    MoveList *pBomb = NULL;
    MoveList *pEat = NULL;
    int sum = 0;
    int tmp;

    for(p=pMove;;p=p->pPre)
    {
        if( p->move.extra_info )
        {
            sum += p->percent*p->value;
            goto loop;
        }

        if( p->move.result==KILLED )
        {
            pKill = p;
        }
        else if( p->move.result==BOMB )
        {
            pBomb = p;
        }
        else if( p->move.result==EAT )
        {
            pEat = p;
        }
loop:
        if( memcmp(&p->move,&p->pPre->move,4) || p->isHead )
        {
            break;
        }
    }

    if( pKill!=NULL )
    {
        if( !bSet)
        {
            tmp = pKill->percent>>1;
        }
        else
        {
            tmp = 0;
        }
        if( pBomb!=NULL )
        {
            pKill->percent -= tmp;
            if( pEat!=NULL )
            {
                pBomb->percent += tmp>>1;
                pEat->percent += tmp>>1;
                sum += pKill->percent*pKill->value+
                        pBomb->percent*pBomb->value+
                        pEat->percent*pEat->value;
            }
            else
            {
                pBomb->percent += tmp;
                sum += pKill->percent*pKill->value+
                        pBomb->percent*pBomb->value;
            }
        }
        else if( pEat!=NULL )
        {
            pKill->percent -= tmp;
            pEat->percent += tmp;
            sum += pKill->percent*pKill->value+
                    pEat->percent*pEat->value;
        }
        else
        {
            sum = alpha<<8;
        }
    }
    else
    {
        sum = alpha<<8;
    }

    return sum;
}

int AdjustHashMove(MoveHash *pHash, MoveList *pMove, int alpha)
{
    int val1;
    int val2;
    int value = 0;

    val1 = GetHashMoveValue(pHash->pMove,alpha,pHash->bSet);
    val2 = GetHashMoveValue(pMove,alpha,0);
    pHash->bSet = 1;
//    log_a("adjust:");
//    SafeMemout((u8*)&pHash->pMove->move, sizeof(MoveResultData));
//    SafeMemout((u8*)&pMove->move, sizeof(MoveResultData));
    if( val1<val2 )
    {
        pHash->pMove = pMove;
        value = val2>>8;
    }
    else
    {
        value = val1>>8;
    }

    return value;
}

void FreeMoveHashNode(Junqi *pJunqi,  MoveList *pMove)
{
    MoveHash *p;
    MoveHash *pPre;
    Engine *pEngine = pJunqi->pEngine;
    MoveHash **paHash = pJunqi->paHash;
    int h;
    int iKey;

    if( paHash==NULL )
    {
        return;
    }
    if( !pMove->keyFlag )
    {
        return;
    }
    iKey = pMove->iKey;
    h = iKey&1023;
    pPre = paHash[h];
    for(p=pPre; p!=NULL; p=p->pNext)
    {
        if( p->iKey==iKey )
        {
            if( pJunqi->cnt==p->cnt )
            {
                if( paHash[h]==p )
                {
                    paHash[h] = p->pNext;
                }
                else
                {
                    pPre->pNext = p->pNext;
                }
                free_cnt++;
                memsys5Free(pJunqi,p);
                break;
            }

        }
        pPre = p;
    }
}

u8 NotRecordMoveHash(Junqi *pJunqi,MoveList *pMove)
{
    u8 rc = 0;
    BoardChess *pSrc;
    //BoardChess *pDst;
    BoardPoint p1;
   // BoardPoint p2;

    p1.x = pMove->move.src[0]%17;
    p1.y = pMove->move.src[1]%17;
//    p2.x = pMove->move.dst[0]%17;
//    p2.y = pMove->move.dst[1]%17;

    //pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;
    pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;


    if( !pSrc->pLineup->isNotLand )
    {
        rc = 1;
    }
    return rc;
}

int RecordMoveHash(
        Junqi *pJunqi,
        MoveHash ***paHash,
        MoveList *pMove,
        int alpha,
        u8 isEat )
{
    Engine *pEngine = pJunqi->pEngine;
    int nHash = 1024;
    int size;
    int h;
    int rc = 0;
    MoveHash *pNew;
    MoveHash *p;
    int iKey;

    if( !isEat && NotRecordMoveHash(pJunqi,pMove) )
    {
        return alpha;
    }
    if( isEat )
    {
        pEngine->eatInList = 1;
        pEngine->eatIndex = 0;
    }
    if( *paHash==NULL )
    {
        size = sizeof(MoveHash *)*nHash;
        *paHash = (MoveHash **)memsys5Malloc(pJunqi,size);
        memset(*paHash,0,size);
    }
    iKey = AddMoveToHash(pJunqi, pMove, isEat);
    pMove->iKey = iKey;
    pMove->keyFlag = 1;
   // assert(pMove->iKey>0);

    h = iKey&1023;
    if( (*paHash)[h]==NULL )
    {
        size =sizeof(MoveHash);
        malloc_cnt++;
        pNew = (MoveHash *)memsys5Malloc(pJunqi,size);
        memset(pNew,0,size);
        pNew->iKey = iKey;
        pNew->cnt = pJunqi->cnt;
        pNew->iDir = pJunqi->eTurn;
        pNew->eatFlag = isEat;
        pNew->pMove = pMove;
        (*paHash)[h] = pNew;
        //log_a("malloc key %d",pNew->iKey);

    }
    else
    {
        if( !isEat )
        {
            for(p=(*paHash)[h]; p!=NULL; p=p->pNext)
            {
                if( p->iKey==iKey )
                {
                   // if( pJunqi->cnt==p->depth && !memcmp(p->pMove->move.dst,pMove->move.dst,2) )
                    rc = 1;
                    alpha = AdjustHashMove(p,pMove,alpha);
                    break;

                }
            }
        }
        if( !rc )
        {
            size =sizeof(MoveHash);
            malloc_cnt++;
            pNew = (MoveHash *)memsys5Malloc(pJunqi,size);
            memset(pNew,0,size);
            pNew->iKey = iKey;
            pNew->cnt = pJunqi->cnt;
            pNew->iDir = pJunqi->eTurn;
            pNew->eatFlag = isEat;
            pNew->pMove = pMove;
            //log_a("malloc key %d",pNew->iKey);
            pNew->pNext = (*paHash)[h];
            (*paHash)[h] = pNew;
        }
    }
    return alpha;

}

void ClearMoveHash(Junqi *pJunqi,MoveHash ***paHash)
{

    MoveHash *pDel;
    MoveHash *pTemp;

    if( *paHash==NULL )
    {
        return;
    }
    for(int i=0; i<1024; i++)
    {

        for(pDel=(*paHash)[i];pDel!=NULL;)
        {
            pTemp = pDel;
           // log_a("free key %d",pTemp->iKey);
            pDel = pDel->pNext;
            free_cnt++;
            memsys5Free(pJunqi,pTemp);

        }
        (*paHash)[i] = NULL;

    }
    memsys5Free(pJunqi,*paHash);
    *paHash = NULL;
}


void SetBestMoveNode(
        BestMoveList *pList,
        MoveList *pMove,
        MoveList *pMaxMove )
{

    MoveList *p;
    int type;
    int i;


  //  aBestMove[cnt-1].flag2 = 1;
    pList->isMove = 1;

    memset(pList->result,0,sizeof(pList->result));


//    prePercent = nowPercent = pMove->percent;
//    pList->index = pMove->move.result-1;
    pList->index = 20;//test
    for(p=pMove;;p=p->pPre)
    {
        type =  p->move.result-1;//type类型从1开始，减1为了节省空间
     //   nowPercent = p->percent;
        if( pList->result[type].flag )
        {
            for(i=RESULT_NUM-1; i>0; i--)
            {
                if( !pList->result[i].flag )
                {
                    type = i;
                    break;
                }
            }
        }

        pList->result[type].flag = 1;
        memcpy(&pList->result[type].move,&p->move,sizeof(MoveResultData));
        pList->result[type].percent = p->percent;
        pList->result[type].value = p->value;


        if( pMaxMove==p )
        {
            pList->index = type;
        }
//        if( (pretype==KILLED-1) || nowPercent>prePercent )
//        {
//            pList->index = type;//从后往前
//        }
//        pretype = type;
//        prePercent = nowPercent;

       // SafeMemout((u8*)&p->move, sizeof(p->move));
        if( memcmp(&p->move,&p->pPre->move,4) || p->isHead )
        {
            break;
        }
    }


    assert(pList->index<6);
}

void UpdateBestList(
        Junqi *pJunqi,
        BestMoveList *pDst,
        BestMoveList *pSrc,
        u8 isShare)
{

    BestMoveList *p;
    BestMoveList *p1;

    p = pDst;
    assert(p!=NULL);

    if( pJunqi->cnt>BEST_LIST_NUM-1 )
    {
        return;
    }

    for(p1=pSrc; p1!=NULL&&p1->isMove; p1=p1->pNext)
    {
        if(p->pNext==NULL)
        {
            if( isShare )
            {
                p->pNext = (BestMoveList*)malloc(sizeof(BestMoveList));
            }
            else
            {
                p->pNext = (BestMoveList*)memsys5Malloc(pJunqi, sizeof(BestMoveList));
            }
            memset(p->pNext, 0, sizeof(BestMoveList));
        }
        memcpy(p->pNext->result,p1->result,sizeof(p1->result));
        p->pNext->index = p1->index;
        p->pNext->isMove = 1;
        p = p->pNext;
    }
    if( p->pNext!=NULL  )
    {
        p->pNext->isMove = 0;
    }
}

void UpdateBestMove(
        Junqi *pJunqi,
        BestMove *aBestMove,
        MoveList *pMove,
        MoveList *pMaxMove,
        int depth,
        int cnt)
{
    BestMoveList *p;

    if( cnt>BEST_LIST_NUM )
    {
        return;
    }
    if( aBestMove[cnt-1].pHead==NULL )
    {
       // aBestMove[cnt-1].pHead = (BestMoveList*)malloc(sizeof(BestMoveList));
        aBestMove[cnt-1].pHead = (BestMoveList*)memsys5Malloc(pJunqi,sizeof(BestMoveList));
        memset(aBestMove[cnt-1].pHead, 0, sizeof(BestMoveList));

    }
    p = aBestMove[cnt-1].pHead;


    SetBestMoveNode(p,pMove,pMaxMove);

   // if( aBestMove[cnt].pHead->isMove )//下一层不是无棋可走
    {
        UpdateBestList(pJunqi,aBestMove[cnt-1].pHead,aBestMove[cnt].pHead,0);
    }

}

void FreeBestMoveList(
        Junqi *pJunqi,
        BestMove *aBeasMove,
        int depth)
{

    BestMoveList *pTemp;
    BestMoveList *p;
    for(int i=0; i<BEST_LIST_NUM; i++)
    {
        for(p=aBeasMove[i].pHead;p!=NULL; )
        {
            pTemp = p;
            p=p->pNext;
            //free(pTemp);
            memsys5Free(pJunqi,pTemp);
        }
    }
}

void FreeSortMoveNode(Junqi *pJunqi, BestMoveList *pNode)
{
    BestMoveList *pTemp;
    BestMoveList *p;
    for(p=pNode;p!=NULL; )
    {

        //SafeMemout((u8 *)&p->result[p->index].move,4);

        pTemp = p;
        p=p->pNext;
        free(pTemp);
        //memsys5Free(pJunqi,pTemp);
    }
}

int GetMaxPerMove(MoveResult *result)
{
    int mxPerCent;
    int index = 0;

    if( result[0].move.result!=MOVE )
    {
        mxPerCent = result[0].percent;
        for(int i=0; i<RESULT_NUM; i++)
        {
            if( result[i].percent>mxPerCent )
            {
                mxPerCent = result[i].percent;
                index = i;
            }
        }
    }
    return index;
}

void PrintBestMove(BestMove *aBestMove, int alpha)
{
    BestMoveList *pHead = aBestMove[0].pHead;
    BestMoveList *p;
    int i = 0;
    for(p=pHead; p!=NULL&&p->isMove; p=p->pNext)
    {
        i++;


        for(int j=0;j<RESULT_NUM;j++)
        {
            if( !p->result[j].flag ) continue;
            log_a("depth %d val %d per %d",i,p->result[j].value,p->result[j].percent);

            SafeMemout((u8*)&p->result[j].move, sizeof(MoveResultData));
        }
//        if( !aBestMove[i].pHead->isMove )
//        {
//            //即使这样也并不一定完全过滤
//            //下一层搜到的最佳着法不一定能对当前层更新
//            break;
//        }
    }
    log_a("avr val: %d",alpha);
}

//目前来看这段代码的性价比实在太低
//花了大量时间调试，得到的性能提升很少
//好处是超时后优先得到上一层的最佳着法
int SearchBestMove(
        Junqi *pJunqi,
        BestMove *aBestMove,
        AlphaBetaData *pData,
        MoveList *pBest ,
        int flag
        )
{
    int sum = 0;
    int mxVal = -INFINITY;
    MoveList temp;
    memset(&temp,0,sizeof(temp));
    int val;
    int depth = pData->depth;
    int alpha = pData->alpha;
    int beta = pData->beta;
    int cnt = pData->cnt;
    int mxPerMove;
    int iDir = pJunqi->eTurn;
    BestMoveList *pNode = aBestMove[0].pNode;
    u8 preventFlag = 0;
   // return mxVal;
    //是否是最后一层，上一层是否概率最大，这一层是否被搜索过
    if( !aBestMove[cnt-1].flag1 && pNode!=NULL &&
            aBestMove[cnt-1].mxPerFlag &&
            pJunqi->cnt<=pJunqi->nDepth )
    {
        if( !pNode->isMove )
        {
            goto end_best;
        }
        //mxPerMove = GetMaxPerMove(pNode->result);

        mxPerMove = pNode->index;
        memcpy(&pBest->move,&pNode->result[mxPerMove].move,sizeof(MoveResultData));
        pData->hasBest = 1;
        for(int i=0; i<RESULT_NUM; i++)
        {
            pJunqi->eTurn = iDir;
            if( !pNode->result[i].flag ) continue;

            aBestMove[cnt-1].pTest = &temp;
            memcpy(&aBestMove[cnt-1].pTest->move,
                    &pNode->result[i].move,
                    sizeof(MoveResultData));
            aBestMove[cnt-1].pTest->percent = pNode->result[i].percent;

            if( i==mxPerMove )
            //if( (MOVE-1)==i )//去掉多分支搜索，减少调试负担
            {
                //assert(pNode==aBestMove[0].pNode);
                aBestMove[cnt].mxPerFlag = 1;
                aBestMove[cnt].mxPerFlag1 = 1;
                aBestMove[0].pNode = aBestMove[0].pNode->pNext;
                //todo RecordMoveHash
            }
            else
            {
                aBestMove[cnt].mxPerFlag = 0;
                aBestMove[cnt].mxPerFlag1 = 0;
            }
            MakeNextMove(pJunqi,&pNode->result[i].move);

            if( pJunqi->aInfo[pJunqi->eTurn].bDead )
            {
                //敌方2家都阵亡后避免继续搜索
                depth = 1;
            }

            if( pNode->result[i].move.result==EAT )
            {
                pData->hasEat = 1;
                val = RecordMoveHash(pJunqi,&pJunqi->paHash,pBest,val,1);
                if( pNode->result[i].percent==256 )
                {
                    pBest->percent = 256;
                }
            }
            CheckPreventMove(pJunqi,pBest,&preventFlag);

            if( pNode->result[i].move.result==MOVE )
            {
                if( !flag )
                    val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir,1);
                else
                    val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,iDir,1);
            }
            else
            {
                //碰撞中有3种情况，不能截断
                if( !flag )
                    val = CallAlphaBeta(pJunqi,depth-1,alpha,INFINITY,iDir,0);
                else
                    val = CallAlphaBeta1(pJunqi,depth-1,alpha,INFINITY,iDir,0);
            }

            if( pNode->result[i].move.result==EAT )
            {
                FreeMoveHashNode(pJunqi,pBest);
            }
            if( preventFlag )
            {
                pJunqi->gFlag[FLAG_PREVENT] = 0;
            }

            pNode->result[i].value = val;

            if( i>0 )
                sum += val*pNode->result[i].percent;

//            if(cnt>1&&!memcmp(&aBeasMove[0].pTest->move,test4,4)&&
//                    !memcmp(&aBeasMove[1].pTest->move,test3,4))

            //if(cnt==1&&!memcmp(aBeasMove[0].pTest,test2,4) )
            {

                log_a("best cnt %d val %d per %d",cnt,val,aBestMove[cnt-1].pTest->percent);
                SafeMemout((u8*)&aBestMove[cnt-1].pTest->move, sizeof(MoveResultData));

            }

            UnMakeMove(pJunqi,&pNode->result[i].move);
        }
        if( !pNode->result[MOVE-1].flag )
        {
            val = sum>>8;
        }
        if( val>mxVal )
        {
            mxVal = val;

            if( val>alpha )
            {
//                assert( alpha==-INFINITY );
//                assert(depth!=1);
                alpha = val;
                memcpy(aBestMove[cnt-1].pHead->result,pNode->result,sizeof(pNode->result));
                aBestMove[cnt-1].pHead->index = pNode->index;
                aBestMove[cnt-1].pHead->isMove = pNode->isMove;
                UpdateBestList(pJunqi, aBestMove[cnt-1].pHead,aBestMove[cnt].pHead,0);
                log_a("alpha1: %d depth %d",alpha,cnt);

            }
        }

        if( 1==cnt )
        {
           // aBestMove[0].pHead->index = mxPerMove;
            if( !pJunqi->gFlag[TIME_OUT] )
            {
                AddMoveSortList(pJunqi,aBestMove,aBestMove[0].pHead,NULL,val,1);
            }

            PrintBestMove(aBestMove,val);
            log_a("end %d",val);
        }
end_best:
        aBestMove[cnt-1].flag1 = 1;

    }

    return mxVal;
}

//此函数放弃维护

int AlphaBeta(
		Junqi *pJunqi,
		int depth,
		int alpha,
		int beta)
{
#if 0
	MoveList *pHead;
	MoveList *p;
	MoveList *pMaxMove;

	MoveResultData *pData;
	MoveResultData *pBest = NULL;
	MoveResultData xInitBest;
	int val;
	int sum = 0;
	static int cnt = 0;
	int iDir = pJunqi->eTurn;

	BestMove *aBestMove = pJunqi->pEngine->aBestMove;
	int mxVal = -INFINITY;
	int mxPercent;




//    u8 test1[4] = {6,11,5,10};
//    u8 test2[4] = {10,5,11,6};
//    u8 test3[4] = {12,6,10,4};
//    u8 test4[4] = {5,10,8,10};

	cnt++;
	if( 1==cnt )
	{

	    //pJunqi->paHash = NULL;
	    aBestMove[0].mxPerFlag = 1;
	    aBestMove[0].mxPerFlag1 = 1;
	    aBestMove[0].pNode = aBestMove[0].pHead;
	}


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


	QueryPerformanceCounter(&nBeginTime);
	//生成着法列表
	pHead = GenerateMoveList(pJunqi, iDir);
    QueryPerformanceCounter(&nEndTime);
    pJunqi->test_time[0] = nEndTime.QuadPart-nBeginTime.QuadPart;
    pJunqi->test_time[1] += pJunqi->test_time[0];


	//无棋可走时直接跳到下一层
	if( NULL==pHead )
	{
		pJunqi->eTurn = iDir;
		ChessTurn(pJunqi);
		mxVal = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir,1);

        //不要再搜下一层了
        //例如3层搜索时，刚开始是最好着法是3步棋
        //后来搜到2步棋的最好着法，第3步无棋可走
        //由于代码原因，在链表里的第3步仍然还在
        //第4层搜索时不能把这一步搜进去
        //*********************************
        //在SearchBestMove里，一层可能有多种碰撞结果
        //不是概率最大的一种，不要修改pNode
        //因为best的搜索只在概率最大的分支下面
        if( aBestMove[cnt-1].mxPerFlag )
        {
            aBestMove[0].pNode = NULL;
            aBestMove[cnt-1].flag2 = 0;
        }
	}
	else
	{
	    mxVal = SearchBestMove(pJunqi,aBestMove,cnt,alpha,beta,&pBest,depth,0);
	    if( pBest!=NULL )
	    {
	        memcpy(&xInitBest, pBest, sizeof(MoveResultData) );
	    }
	}


	if( mxVal>alpha ) alpha = mxVal;

    //遍历每一个着法
    aBestMove[cnt].mxPerFlag1 = 1;
    pMaxMove = pHead;
    mxPercent = 0;
    sum = 0;

    for(p=pHead; pHead!=NULL; p=p->pNext)
    {

    	pJunqi->eTurn = iDir;

    	if( p->move.result==MOVE )
    	{
    	    mxPercent = 0;
    	    aBestMove[cnt].mxPerFlag1 = 1;
    	}
    	//模拟着法产生后的局面

    	MakeNextMove(pJunqi,&p->move);


        if ( pBest!=NULL && !memcmp(&xInitBest, &p->move, 4) )
        {
            UnMakeMove(pJunqi,&p->move);
            goto continue_search;
        }
    	else
    	{
    	    aBestMove[cnt-1].pTest = p;
            if( p->move.result==MOVE )
            {
                val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir,1);
            }
            else
            {
                //碰撞中有3种情况，不能截断
                val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir,0);
            }
            p->value = val;


//            if(cnt>1&&!memcmp(&aBeasMove[0].pTest->move,test4,4)&&
//                    !memcmp(&aBeasMove[1].pTest->move,test3,4))
//
           // if( cnt==1||cnt==2 )
//            if( cnt==1 )
//            {
//                log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                SafeMemout((u8*)&p->move, sizeof(p->move));
//            }


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
    		//log_a("sum %d val %d per %d",sum,val,p->percent);
    		pData = &p->pNext->move;
            //下一个着法
    		if( memcmp(&p->move, pData, 4) || p->pNext->isHead  )
    		{
    		    pMaxMove = p->pNext;
    		    aBestMove[cnt].mxPerFlag1 = 1;
    			val = sum>>8;
    			sum = 0;
    		}
    		else
    		{
    		    //标记最大概率的碰撞结果
    		    //有3种结果，第一种始终置1
    		    //此后如果概率非最大值置0
                if( p->percent>mxPercent )
                {
                    mxPercent = p->percent;
                }
                if( p->pNext->percent>mxPercent )
                {
                    pMaxMove = p->pNext;
                    aBestMove[cnt].mxPerFlag1 = 1;
                }
                else
                {
                    aBestMove[cnt].mxPerFlag1 = 0;
                }
    		    goto continue_search;
    		}
    	}
        if( cnt==1 )
        {
            AddMoveSortList(pJunqi,aBestMove,p,pMaxMove,val,0);
        }
        //产生截断
    	if( val>=beta )
    	{
    	    if( -INFINITY==mxVal && aBestMove[cnt-1].mxPerFlag1 )
    	    {
    	        UpdateBestMove(pJunqi,aBestMove,p,pMaxMove,depth,cnt);
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val);
                }
                pBest = &p->move;
    	    }
    		alpha = val;
    		mxVal = val;
    		break;
    	}
    	//更新alpha值
        if( val>mxVal )
        {
            mxVal = val;
            if( aBestMove[cnt-1].mxPerFlag1 )
            {
                //log_a("best");
                UpdateBestMove(pJunqi,aBestMove,p,pMaxMove,depth,cnt);
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val);
                }
                pBest = &p->move;
            }
            if( val>alpha )
            {

                alpha = val;
//                log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                log_a("alpha: %d depth %d",alpha,cnt);
//                SafeMemout((u8*)&p->move, sizeof(p->move));

            }
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


    }

    cnt--;
    if( 0==cnt )
    {
    	cnt = 0;
    	SetBestMove(pJunqi,pBest);
    }
	ClearMoveList(pJunqi,pHead);

	return mxVal;
#else
	return 0;
#endif

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





