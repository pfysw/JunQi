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

int free_cnt = 0;
int malloc_cnt = 0;
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
	if( pJunqi->bMove || pJunqi->bGo || search_time>2000 )
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

int CheckMoveHash(MoveHash ***paHash, int iKey, int depth, int iDir)
{
    int h;
    int rc = 0;
    MoveHash *p;

    if( *paHash==NULL )
    {
        return 0;
    }
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
                //当h等于0时，防止返回0
                if( ((p->iDir&1)==(iDir&1)) && p->depth>=depth )
                {
                    rc = h+1;
                }
                break;
            }
        }
    }

    return rc;
}


void RecordMoveHash(
        MoveHash ***paHash,
        int iKey,
        u8 iDir,
        u8 depth,
        int val)
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
        *paHash = (MoveHash **)malloc(size);
        memset(*paHash,0,size);
    }
    h = iKey&1023;
    if( (*paHash)[h]==NULL )
    {
        size =sizeof(MoveHash);
        malloc_cnt++;
        pNew = (MoveHash *)malloc(size);
        memset(pNew,0,size);
        pNew->iKey = iKey;
        pNew->depth = depth;
        pNew->value = val;
        pNew->iDir = iDir;
        (*paHash)[h] = pNew;
        //log_a("malloc key %d",pNew->iKey);

    }
    else
    {
        for(p=(*paHash)[h]; p!=NULL; p=p->pNext)
        {
            if( p->iKey==iKey )
            {
                rc = 1;
                if( depth>p->depth && (p->iDir&1)==(iDir&1) )
                {
                    p->depth = depth;
                    p->value = val;
                    p->iDir = iDir;
                }
                break;
            }
        }
        if( !rc )
        {
            size =sizeof(MoveHash);
            malloc_cnt++;
            pNew = (MoveHash *)malloc(size);
            memset(pNew,0,size);
            pNew->iKey = iKey;
            pNew->depth = depth;
            pNew->value = val;
            pNew->iDir = iDir;
            //log_a("malloc key %d",pNew->iKey);
            pNew->pNext = (*paHash)[h];
            (*paHash)[h] = pNew;
        }
    }

}

void ClearMoveHash(MoveHash ***paHash)
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
            free(pTemp);

        }
        (*paHash)[i] = NULL;

    }
    free(*paHash);
    *paHash = NULL;
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


void SetBestMoveNode(
        BestMove *aBeasMove,
        BestMoveList *pList,
        MoveList *pMove,
        int depth,
        int cnt )
{

    MoveList *p;
    int type;

    aBeasMove[cnt-1].flag2 = 1;
    memset(pList->result,0,sizeof(pList->result));
    for(p=pMove;;p=p->pPre)
    {
        type =  p->move.result;
        pList->result[type-1].flag = 1;
        //type类型从1开始，减1为了节省空间
        memcpy(&pList->result[type-1].move,&p->move,sizeof(MoveResultData));
        pList->result[type-1].percent = p->percent;
       // SafeMemout((u8*)&p->move, sizeof(p->move));
        if( memcmp(&p->move,&p->pPre->move,4) || p->isHead )
        {
            break;
        }
    }
}

void UpdateBestMove(
        BestMove *aBeasMove,
        MoveList *pMove,
        int depth,
        int cnt,
        u8 isHashVal)
{
    BestMoveList *p;
    BestMoveList *p1;

    if( aBeasMove[cnt-1].pHead==NULL )
    {
        aBeasMove[cnt-1].pHead = (BestMoveList*)malloc(sizeof(BestMoveList));
        memset(aBeasMove[cnt-1].pHead, 0, sizeof(BestMoveList));

    }
    p = aBeasMove[cnt-1].pHead;
    SetBestMoveNode(aBeasMove,p,pMove,depth,cnt);


#ifdef MOVE_HASH
    int i;
    //因为这个最佳变例是从hash取得的
    //所以不知道后续的最佳着法是什么，将flag2清0
    if( isHashVal )
    {
        i = cnt;
        for(p1=aBeasMove[cnt].pHead; p1!=NULL; p1=p1->pNext)
        {
            aBeasMove[i++].flag2 = 0;
        }
    }
    else
#endif
    {
        for(p1=aBeasMove[cnt].pHead; p1!=NULL; p1=p1->pNext)
        {
            if(p->pNext==NULL)
            {
                p->pNext = (BestMoveList*)malloc(sizeof(BestMoveList));
                memset(p->pNext, 0, sizeof(BestMoveList));
            }
            memcpy(p->pNext->result,p1->result,sizeof(p1->result));
            p = p->pNext;
        }
    }

}

void FreeBestMoveList(
        BestMove *aBeasMove,
        int depth)
{

    BestMoveList *pTemp;
    BestMoveList *p;
    for(int i=0; i<depth; i++)
    {
        for(p=aBeasMove[i].pHead;p!=NULL; )
        {
            pTemp = p;
            p=p->pNext;
            free(pTemp);
        }
    }
}

int GetMaxPerMove(MoveResult *result)
{
    int mxPerCent;
    int index = 0;

    if( result[0].move.result!=MOVE )
    {
        mxPerCent = result[0].percent;
        for(int i=0; i<4; i++)
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

void PrintBestMove(BestMove *aBestMove, int alpha, int depth)
{
    BestMoveList *p;
    int i = 0;
    for(p=aBestMove[0].pHead; p!=NULL; p=p->pNext)
    {
        i++;
        for(int j=0;j<4;j++)
        {
            if( !p->result[j].flag ) continue;
            log_a("depth %d val %d per %d",i,alpha,p->result[j].percent);
            SafeMemout((u8*)&p->result[j].move, sizeof(MoveResultData));
        }
    }
}

int SearchBestMove(
        Junqi *pJunqi,
        BestMove *aBestMove,
        int cnt,
        int alpha,
        int beta,
        MoveResultData **ppBest ,
        int depth,
        int flag
        )
{
    int sum = 0;
    int mxVal = -INFINITY;
    MoveList temp;
    memset(&temp,0,sizeof(temp));
    int val;

    int mxPerMove;
    int iDir = pJunqi->eTurn;
    BestMoveList *pNode = aBestMove[0].pNode;
    BestMoveList *pPre;

    if( aBestMove[cnt-1].flag2 && aBestMove[cnt-1].mxPerFlag && !aBestMove[cnt-1].flag1 )
    {
        mxPerMove = GetMaxPerMove(pNode->result);
        for(int i=0; i<4; i++)
        {
            pJunqi->eTurn = iDir;
            if( !pNode->result[i].flag ) continue;

            aBestMove[cnt-1].pTest = &temp;
            memcpy(&aBestMove[cnt-1].pTest->move,
                    &pNode->result[i].move,
                    sizeof(MoveResultData));
            aBestMove[cnt-1].pTest->percent = pNode->result[i].percent;

            *ppBest = &pNode->result[i].move;
            if( i==mxPerMove )
            {
                aBestMove[cnt].mxPerFlag = 1;
                aBestMove[cnt].mxPerFlag1 = 1;
                pPre = aBestMove[0].pNode;
                aBestMove[0].pNode = aBestMove[0].pNode->pNext;

            }
            else
            {
                aBestMove[cnt].mxPerFlag = 0;
                aBestMove[cnt].mxPerFlag1 = 0;
            }
//            log_a("mxPer 1:%d 2:%d 3:%d 4:%d",aBestMove[0].mxPerFlag,
//                    aBestMove[1].mxPerFlag,aBestMove[2].mxPerFlag,
//                    aBestMove[3].mxPerFlag );
//            log_a("bestt cnt %d i %d mxPerMove %d per %d key %d ",
//                    cnt,i,mxPerMove,aBestMove[cnt-1].pTest->percent,iKey);
//            SafeMemout((u8*)&aBestMove[cnt-1].pTest->move, sizeof(MoveResultData));

            MakeNextMove(pJunqi,&pNode->result[i].move);
#ifdef MOVE_HASH
            int iKey;
            iKey = GetHashKey(pJunqi);
#endif
            if( !flag )
                val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir);
            else
                val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,iDir);

            if( i>0 )
                sum += val*pNode->result[i].percent;
#ifdef MOVE_HASH
            else
                RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
#endif

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
            if( aBestMove[0].pNode==NULL )
            {
                log_a("NULL");

                pPre->pNext = (BestMoveList*)malloc(sizeof(BestMoveList));
                memset(pPre->pNext, 0, sizeof(BestMoveList));
                assert(aBestMove[cnt].pHead->pNext==NULL);
                memcpy(pPre->pNext->result,aBestMove[cnt].pHead->result,
                        sizeof(pPre->pNext->result));
                aBestMove[0].pNode = pPre->pNext;
            }

            if( val>alpha )
            {
                alpha = val;

                log_a("alpha1: %d depth %d",alpha,cnt);
            }
        }


        if( 1==cnt )
        {
            PrintBestMove(aBestMove,alpha,depth);
            log_a("end");
        }

    }

    aBestMove[cnt-1].flag1 = 1;

    return mxVal;
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

	BestMove *aBestMove = pJunqi->pEngine->aBestMove;
	int new = 0;
	int mxVal = -INFINITY;
	int mxPercent;
#ifdef MOVE_HASH
	int h;
	int iKey;
#endif
	u8 isHashVal;


//    u8 test1[4] = {6,11,5,10};
//    u8 test2[4] = {10,5,11,6};
//    u8 test3[4] = {12,6,10,4};
//    u8 test4[4] = {5,10,8,10};

	cnt++;
	if( 1==cnt )
	{
#ifdef MOVE_HASH
	    pJunqi->paHash = NULL;
#endif
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

//	if(depth==2)
//	log_a("alpha %d beta %d depth %d",alpha,beta,cnt);
	//生成着法列表
	pHead = GenerateMoveList(pJunqi, iDir);

	//无棋可走时直接跳到下一层
	if( NULL==pHead )
	{
		pJunqi->eTurn = iDir;
		ChessTurn(pJunqi);

		val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir);
	}

	mxVal = SearchBestMove(pJunqi,aBestMove,cnt,alpha,beta,&pBest,depth,0);

	if( mxVal>alpha ) alpha = mxVal;

    //遍历每一个着法
    aBestMove[cnt].mxPerFlag1 = 1;
    mxPercent = 0;
    sum = 0;
    isHashVal = 0;
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
#ifdef MOVE_HASH
    	assert(pJunqi->pEngine->pPos!=NULL);
//        LARGE_INTEGER nBeginTime;
//        LARGE_INTEGER nEndTime;
//        QueryPerformanceCounter(&nBeginTime);
        iKey = GetHashKey(pJunqi);
//        QueryPerformanceCounter(&nEndTime);
//        pJunqi->test_time[0] = nEndTime.QuadPart-nBeginTime.QuadPart;
//        pJunqi->test_time[1] += pJunqi->test_time[0];
#endif

        if ( pBest!=NULL && !memcmp(pBest, &p->move, 4) )
        {
            UnMakeMove(pJunqi,&p->move);
            goto continue_search;
        }
#ifdef MOVE_HASH
        else if( ( p->move.result==MOVE &&
    	      (h = CheckMoveHash(&pJunqi->paHash,iKey,depth,pJunqi->eTurn)) ) )
      //  else if( p->move.result==MOVE && CheckMoveHash1(&pJunqi->paHash,iKey) )
    	{
//            log_a("repeat key %d",iKey);
//            SafeMemout((u8*)&p->move, sizeof(p->move));


//            if( cnt==1||cnt==2 )
//            {
//               // log_a("hash %d",h,pJunqi->paHash[h-1]->iKey);
//                log_a("重复 cnt %d val %d per %d key %d",cnt,val,p->percent,iKey);
//                SafeMemout((u8*)&p->move, sizeof(p->move));
//            }

            isHashVal = 1;
    	    val = pJunqi->paHash[h-1]->value;
    	    UnMakeMove(pJunqi,&p->move);
    	    //goto continue_search;
    	}
#endif
    	else
    	{

    	    aBestMove[cnt-1].pTest = p;
            val = CallAlphaBeta(pJunqi,depth-1,alpha,beta,iDir);
#ifdef MOVE_HASH
            RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
#endif
           // RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
//            if( p->move.result==MOVE )
//            {
//                RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
//            }

//            if(cnt==3&&!memcmp(aBeasMove[0].pTest,test4,4)&&
//                    !memcmp(aBeasMove[1].pTest,test3,4) &&
//                    !memcmp(aBeasMove[2].pTest,test1,4) )
//            {
//                for(int i=0;i<3;i++)
//                {
//                log_a("cnt %d val %d per %d",i,val,aBeasMove[i].pTest->percent);
//                SafeMemout((u8*)&aBeasMove[i].pTest->move, sizeof(p->move));
//                }
//            }
//
//            if(cnt==3)
//                log_c("ds");

//            if(cnt>1&&!memcmp(&aBeasMove[0].pTest->move,test4,4)&&
//                    !memcmp(&aBeasMove[1].pTest->move,test3,4))
//
//            if( cnt==1||cnt==2 )
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
    		    aBestMove[cnt].mxPerFlag = 1;
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
                    aBestMove[cnt].mxPerFlag1 = 1;
                }
                else
                {
                    aBestMove[cnt].mxPerFlag1 = 0;
                }
    		    goto continue_search;
    		}
    	}

        //产生截断
    	if( val>=beta )
    	{
    	    if( -INFINITY==mxVal && aBestMove[cnt-1].mxPerFlag1 )
    	    {
    	        UpdateBestMove(aBestMove,p,depth,cnt,isHashVal);
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
                UpdateBestMove(aBestMove,p,depth,cnt,isHashVal);
            }
            if( val>alpha )
            {
                pBest = &p->move;
                alpha = val;
//                log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                log_a("alpha: %d depth %d",alpha,cnt);
//                SafeMemout((u8*)&p->move, sizeof(p->move));
                new = 1;

            }
        }
        isHashVal = 0;

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

    	if( cnt==1 && new)
    	{
    	    new = 0;
    	    PrintBestMove(aBestMove,alpha,depth);
    	}
    }

    cnt--;
    if( 0==cnt )
    {
    	cnt = 0;
    	SetBestMove(pJunqi,pBest);
#ifdef MOVE_HASH
    	ClearMoveHash(&pJunqi->paHash);
        log_a("malloc %d",malloc_cnt);
        log_a("free %d",free_cnt);
#endif
    }
	ClearMoveList(pHead);


	return mxVal;
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


