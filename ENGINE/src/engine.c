/*
 * engine.c
 *
 *  Created on: Aug 18, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include <time.h>
#include "event.h"
#include "engine.h"
#include "path.h"
#include "evaluate.h"
#include "movegen.h"
#include "search.h"
#include "windows.h"
#include <mqueue.h>


int preTurn = 1000;

EventHandle eventArr[] = {
	{ ProJunqiEvent, MOVE_EVENT },
	{ ProJunqiEvent, JUNQI_EVENT }
};


u32 random_(void)
{
	static u32 x = 0;
	static u8 isInit = 0;
	if(!isInit)
	{
		isInit =1;
		int t = (unsigned int)time(NULL);
		printf("t %d\n",t);
		srand(t);
	}
    for(int i=0; i<20; i++)
    {
    	x ^= (rand()&1)<<i;
    }
    x++;
    return x;
}

void ProMoveEvent(Junqi* pJunqi, u8 iDir, u8 event)
{
	if( event==JUMP_EVENT )
	{
		assert( iDir==pJunqi->eTurn );
		IncJumpCnt(pJunqi, iDir);
		ChessTurn(pJunqi);
	}
	else if( event==SURRENDER_EVENT )
	{
	    if( GenerateMoveList(pJunqi, iDir)==NULL )
	    {
	        //复盘无棋可走并没有记录在棋谱中
	        pJunqi->iRpOfst--;
	    }
		DestroyAllChess(pJunqi, iDir);
		if( iDir==pJunqi->eTurn )
		{
			ChessTurn(pJunqi);
		}
	}
}

Engine *OpneEnigne(Junqi *pJunqi)
{
	Engine *pEngine = (Engine *)malloc(sizeof(Engine));
	memset(pEngine, 0, sizeof(Engine));
	memset(aEventBit, 0, sizeof(aEventBit));
	pEngine->ppMoveSort = (MoveSort **)malloc(sizeof(MoveSort *));
	memset(pEngine->ppMoveSort,0,sizeof(MoveSort *));
	pEngine->pJunqi = pJunqi;
	InitValuePara(&pEngine->valPara);
	return pEngine;
}

void CloseEngine(Engine *pEngine)
{
	if(pEngine!=NULL)
	{
		pEngine->pJunqi->pEngine = NULL;
		free(pEngine);
	}
}

void CheckMoveEvent(Engine *pEngine)
{
	CheckJunqiEvent(pEngine);
}

u8 DealEvent(Engine *pEngine)
{
	u8 isMove = 0;
	int i;
	u8 eventFlag = 0;
	u8 eventId = 0;
	u8 index;

	for(i=0; i<sizeof(eventArr)/sizeof(eventArr[0]); i++)
	{
		if( TESTBIT(aEventBit, eventArr[i].eventId) )
		{
			if( eventArr[i].eventId>=eventId )
			{
				eventId = eventArr[i].eventId;
				index = i;
			}
			eventFlag = 1;
		}
	}

	if( eventFlag )
	{
		isMove = eventArr[index].xEventFun(pEngine);
	}

	return isMove;
}


void ProMoveResult(Junqi* pJunqi, u8 iDir, u8 *data)
{
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;
	MoveResultData *pResult = (MoveResultData*)data;

	p1.x = pResult->src[0]%17;
	p1.y = pResult->src[1]%17;
	p2.x = pResult->dst[0]%17;
	p2.y = pResult->dst[1]%17;
	if( pJunqi->aBoard[p1.x][p1.y].pAdjList && pJunqi->aBoard[p2.x][p2.y].pAdjList )
	{
		pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;
		pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;
		if( pSrc==NULL || pDst==NULL )
		{
			SendHeader(pJunqi, iDir, COMM_ERROR);
			assert(0);
		}
	}
	else
	{
		SendHeader(pJunqi, iDir, COMM_ERROR);
		assert(0);
	}
	if( pResult->result==MOVE )
	{
	    pJunqi->nNoEat++;
	}
	else
	{
	    pJunqi->nEat++;
	    pJunqi->nNoEat = 0;
	}
	//log_c("dir %d %d\n",pSrc->pLineup->iDir,iDir);
	assert( pSrc->pLineup->iDir==iDir );
	PlayResult(pJunqi, pSrc, pDst, pResult);

	if( pJunqi->bStart )
	{
		ChessTurn(pJunqi);
#ifdef  EVENT_TEST
		if( pJunqi->nNoEat>8 )
		{
		    CheckMoveEvent(pJunqi->pEngine);
		}
#endif
	}


}

BoardChess * GetMoveDst(Junqi* pJunqi, BoardChess *pSrc)
{
	BoardChess *pDst=NULL;
	BoardChess *pTemp;
	u32 rand = 0;
	int i,j;

	rand = random_()%129;
	//rand = 109;
	for(i=0; i<129; i++)
	{
		//log_a("aa i %d rand %d k %d",i, rand,(rand+i)%129);
		j = (i+rand)%129;
		if( j<120 )
		{
			assert(j/30>=0&&j/30<4);
			assert(j%30>=0&&j%30<30);
			pTemp = &pJunqi->ChessPos[j/30][j%30];
		}
		else
		{
			assert(j-120>=0&&j-120<9);
			pTemp = &pJunqi->NineGrid[j-120];
		}
		if( pTemp->type!=NONE && pSrc->pLineup->iDir%2==pTemp->pLineup->iDir%2 )
		{
			continue;
		}
		if( IsEnableMove(pJunqi, pSrc,pTemp) )
		{
			pDst = pTemp;
			break;
		}
	}

	return pDst;
}

void SendRandMove(Junqi* pJunqi)
{
    u32 rand;
    int i;
    BoardChess *pSrc;
    BoardChess *pDst;
    ChessLineup *pLineup;

    rand = random_()%30;
    for(i=0;  i<30; i++)
    {
    	pLineup = &pJunqi->Lineup[pJunqi->eTurn][(rand+i)%30];
    	if( pLineup->bDead )
    	{
    		continue;
    	}
    	pSrc = pLineup->pChess;
//    	log_a("i %d rand %d k %d",i, rand,(rand+i)%30);
//    	log_a("type %d",pLineup->type);
    	if(pLineup->type!=NONE && pLineup->type!=JUNQI && pLineup->type!=DILEI )
    	{
    		pDst = GetMoveDst(pJunqi, pSrc);
    		if( pDst!=NULL )
    		{
    			SendMove(pJunqi, pSrc, pDst);
    			return;
    		}
    	}
    }
    SendEvent(pJunqi, pJunqi->eTurn, JUMP_EVENT);

}

void ClearBestMoveFlag(Engine *pEngine)
{
    for(int i=0; i<BEST_LIST_NUM; i++)
    {
        pEngine->aBestMove[i].flag1 = 0;
        //pEngine->aBestMove[i].flag2不能清，代表上一轮搜到的标记
        pEngine->aBestMove[i].mxPerFlag = 0;
        pEngine->aBestMove[i].mxPerFlag1 = 0;
    }
}

void MakeDeepSearch(
        Junqi *pJunqi,
        MoveSort *pNode,
        int depth,
        int type)
{
    Engine *pEngine = pJunqi->pEngine;
    int value;
    int eTurn;

    pJunqi->eSearchType = SEARCH_DEEP;
    if( type==SEARCH_RIGHT )
    {
        pJunqi->eDeepType = SEARCH_LEFT;
        pJunqi->deepTurn = (pJunqi->eTurn+1)%4;
    }
    else if( type==SEARCH_LEFT )
    {
        pJunqi->eDeepType = SEARCH_RIGHT;
        pJunqi->deepTurn = (pJunqi->eTurn+3)%4;
    }
    else
    {
        pJunqi->eDeepType = type;
        eTurn = pJunqi->eTurn;
        ChessTurn(pJunqi);
        pJunqi->deepTurn = pJunqi->eTurn;
        pJunqi->eTurn = eTurn;
    }
    if( !pJunqi->aInfo[pJunqi->deepTurn].bDead )
    {
        //设置记录第一层的行棋
        int index;
        MoveList firdtMove;
        index = pNode->pHead->index;
        memcpy(&firdtMove.move,&pNode->pHead->result[index].move,
                sizeof(MoveResultData));
        pEngine->pFirstMove = &firdtMove;

        value = DeepSearch(pJunqi,pNode->pHead,SEARCH_DEEP,1);
        pNode->aValue[depth][type] = value;
        pNode->isSetValue[depth][type] = 1;
    }

}

void *search_thread(void *arg)
//DWORD WINAPI search_thread(LPVOID arg)
{
    Junqi* pJunqiBase = (Junqi*)arg;
    Junqi* pJunqi;
    Engine* pEngineObj;
    Engine* pEngine;
    int eTurn;
    int value;
    int i;
    int len;
    u8 aBuf[REC_LEN];
    SearchMsg *pMsg;

   //while(!pJunqiBase->test_flag);
    while(1)
    {

        len = mq_receive(pJunqiBase->search_qid, (char *)aBuf, REC_LEN, NULL);
        (void)len;//不用
        pMsg = (SearchMsg*)aBuf;

        pJunqi = (Junqi*)malloc(sizeof(Junqi));
        memcpy(pJunqi,pJunqiBase,sizeof(Junqi));
        pJunqi->pJunqiBase = pJunqiBase;
        //IntiMoveMem(pJunqi);

        memsys5Init(pJunqi,(MEM_POOL_LENGTH>>1),16);

        pEngineObj = (Engine*)malloc(sizeof(Engine));
        memcpy(pEngineObj,pJunqiBase->pEngine,sizeof(Engine));
        pJunqi->pEngine = pEngineObj;
        pEngine = pJunqi->pEngine;

        ChessBoardCopy(pJunqi);
        pthread_mutex_lock(&pJunqi->search_mutex);
        pJunqiBase->cntSearch++;
        pthread_mutex_unlock(&pJunqi->search_mutex);


        pJunqi->eSearchType = pMsg->type;
        pJunqi->gFlag[TIME_OUT] = 0;

        if( pJunqi->eSearchType!=SEARCH_DEEP )
        {
            ProSearch(pJunqi,4);
        }
        else
        {
            MakeDeepSearch(pJunqi,pMsg->pNode,
                    pMsg->deepDepth,pMsg->deepType);
        }

        //结束搜索线程，并释放相关资源
        log_a("malloc d %d",pJunqi->malloc_cnt);
        log_a("free d %d",pJunqi->free_cnt);
        //free(pJunqi->mem_pool.pStart);
        free(pJunqi->pThreadMem);
        ClearAdjNode(pJunqi);
        free(pEngine);
        free(pJunqi);
        log_a("**************************");
        while(!pJunqiBase->begin_flag);
        pJunqiBase->cntSearch--;


    }

    pthread_detach(pthread_self());

    return 0;
}

pthread_t CreatSearchThread(Junqi* pJunqi)
{
    pthread_t tidp;

    pthread_create(&tidp,NULL,(void*)search_thread,pJunqi);
    return tidp;
}

//void CreatSearchThread(Junqi* pJunqi)
//{
//    HANDLE handle1;
//    handle1 = CreateThread(NULL,0,search_thread,pJunqi,0,NULL);
//    //SetThreadAffinityMask(handle1,16);
//}

int ProSearch(Junqi* pJunqi,int depth)
{
    int value;
    int eTurn;
    int i;
    Engine *pEngine = pJunqi->pEngine;

    eTurn = pJunqi->eTurn;

    memset(pEngine->aBestMove,0,sizeof(pEngine->aBestMove));


    for(i=0; i<depth+1; i++)
    {

        //ClearMoveSortList(pJunqi,0);

        pJunqi->eTurn = eTurn;

        pJunqi->test_num = 0;
        pJunqi->test_gen_num = 0;
        pJunqi->searche_num[0] = 0;
        pJunqi->searche_num[1] = 0;
        ClearBestMoveFlag(pEngine);

        //value = AlphaBeta(pJunqi,i,-INFINITY,INFINITY);
        value = AlphaBeta1(pJunqi,i,-INFINITY,INFINITY,1);
        //value = AlphaBetaTest(pJunqi,i,-INFINITY,INFINITY);
        //value = AlphaBetaTest(pJunqi,i,4,5);

        //if( !pJunqi->isDeepSearch )
        {
            log_a("search2 num %d",pJunqi->test_num);
            log_a("gen num %d",pJunqi->test_gen_num);
            log_a("key num %d %d",pJunqi->searche_num[0],
                    pJunqi->searche_num[1]);

            log_a("time %d",time(NULL)-pJunqi->begin_time);
            BoardChess **pBest = pJunqi->pEngine->pBest;
            if(i>0)
            {
                if( *pBest!=NULL )
                {
                    log_a("best %d %d %d %d",pBest[0]->point.x,pBest[0]->point.y,
                            pBest[1]->point.x,pBest[1]->point.y);
                }
                else
                {
                    log_a("no move");
                }
            }


        }
        if( TimeOut(pJunqi) )
        {
            log_a("break");
            break;
        }
        if( eTurn%2!=ENGINE_DIR%2 )
        {
            //value = -value;
            log_a("depth %d value %d",i,-value);
        }
        else
        {
            log_a("depth %d value %d",i,value);
        }


    }

    pJunqi->eTurn = eTurn;
    FreeBestMoveList(pJunqi,pEngine->aBestMove,i);

    return value;
}

u8 IsOnlyTwoDir(Junqi* pJunqi)
{
    u8 rc = 0;
    u8 num = 0;

    for(int i=0; i<4; i++)
    {
        if( pJunqi->aInfo[i].bDead )
        {
            num++;
        }
    }

    if(num>=2)
    {
        rc = 1;
    }

    return rc;
}

void ReSearchInDeep(Junqi* pJunqi, MoveSort *pNode, int depth)
{

    char aBuf[REC_LEN] = {0};
    SearchMsg *pMsg;
    u8 nTread = 0;
    int iDir;

    pJunqi->bSearch = 1;
    pJunqi->bAnalyse = 1;

    pMsg = (SearchMsg*)aBuf;

    pJunqi->malloc_cnt = 0;
    pJunqi->free_cnt = 0;
    pJunqi->bGo = 0;
    pJunqi->bMove = 0;
    pJunqi->gFlag[TIME_OUT] = 0;
    pJunqi->begin_time = (unsigned int)time(NULL);

    pJunqi->begin_flag = 0;
    iDir = pJunqi->eTurn;
    if( !IsOnlyTwoDir(pJunqi) )
    {
        pMsg->type = SEARCH_DEEP;
        pMsg->deepDepth = depth;
        pMsg->pNode = pNode;
        if( !pJunqi->aInfo[(iDir+1)%4].bDead &&
                !pNode->isSetValue[depth][SEARCH_RIGHT] )
        {

            pMsg->deepType = SEARCH_RIGHT;
            mq_send(pJunqi->search_qid, aBuf, sizeof(SearchMsg), 0);
            nTread++;
        }
        if( !pJunqi->aInfo[(iDir+3)%4].bDead &&
                !pNode->isSetValue[depth][SEARCH_LEFT] )
        {
            pMsg->deepType = SEARCH_LEFT;
            mq_send(pJunqi->search_qid, aBuf, sizeof(SearchMsg), 0);
            nTread++;
        }
    }
    while( pJunqi->cntSearch<nTread );//等待所有线程都初始化完毕
    pJunqi->begin_flag = 1;

    if( !pNode->isSetValue[depth][SEARCH_DEFAULT] )
    {
        MakeDeepSearch(pJunqi,pNode,depth,SEARCH_DEFAULT);
    }

    while(pJunqi->cntSearch);//等所有线程搜索完毕
  // MakeDeepSearch(pJunqi,pNode,depth,SEARCH_LEFT);

}


void ProRecMsg(Junqi* pJunqi, u8 *data)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	u8 event;
	u8 isMove = 0;
	char aBuf[REC_LEN] = {0};
	SearchMsg *pMsg;
	u8 nTread = 0;
	int iDir;


	pJunqi->bAnalyse = 0;
	if( memcmp(pHead->aMagic, aMagic, 4)!=0 )
	{
		return;
	}

	switch(pHead->eFun)
	{
	case COMM_EVNET:
		preTurn = pJunqi->eTurn;
		event = *((u8*)&pHead[1]);
		ProMoveEvent(pJunqi, pHead->iDir, event);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_MOVE:
		preTurn = pJunqi->eTurn;
		log_a("turn %d %d",pHead->iDir,pJunqi->eTurn);
		assert( pHead->iDir==pJunqi->eTurn );
		data = (u8*)&pHead[1];

		ProMoveResult(pJunqi, pHead->iDir, data);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_REPLAY:
		log_a("reply %d",pHead->iDir);
		SendHeader(pJunqi, pHead->iDir, COMM_REPLAY);
		pJunqi->eTurn = pHead->iDir;
		pJunqi->bStart = 1;
		//在COMM_START指令中清0
		pJunqi->nRpStep = *((u16*)pHead->reserve);
		pJunqi->iRpOfst = 0;

		//获取复盘布阵，以后可能有用
//		data = (u8*)&pHead[1];
//		InitReplyLineup(pJunqi,&data[8]);

		break;
	default:
		break;
	}

	ReSetBombValue(pJunqi);
	SetMaxType(pJunqi);
	ReSetLineupType(pJunqi);
	EvalSituation(pJunqi,1);//初始化分数

    if(  preTurn<4 )//&& preTurn%2==ENGINE_DIR )
    {
//        static int jj=0;
//        jj++;
//        if(jj==115)
//        log_c("test");
//        log_a("jj %d",jj);

        PrognosisChess(pJunqi,preTurn);

    }

	if( !pJunqi->bStart || pJunqi->bStop )
	{
		return;
	}

    if( 0==pJunqi->nRpStep ||
    	pJunqi->iRpOfst>pJunqi->nRpStep-1 )
    {

        pthread_mutex_lock(&pJunqi->mutex);
        pJunqi->bSearch = 1;
        pJunqi->bAnalyse = 1;
//#define ENG_TEST
#ifndef ENG_TEST
        if( pJunqi->eTurn%2!=ENGINE_DIR )
        {
            goto search_end;
        }
#endif

        pMsg = (SearchMsg*)aBuf;

        pJunqi->malloc_cnt = 0;
        pJunqi->free_cnt = 0;
        pJunqi->bGo = 0;
        pJunqi->bMove = 0;
        pJunqi->gFlag[TIME_OUT] = 0;
        pJunqi->begin_time = (unsigned int)time(NULL);
        pJunqi->cntSearch = 0;



#ifndef ENG_TEST
        pJunqi->begin_flag = 0;
        iDir = pJunqi->eTurn;
        if( !IsOnlyTwoDir(pJunqi) )
        {
            if( !pJunqi->aInfo[(iDir+1)%4].bDead )
            {
                pMsg->type = SEARCH_RIGHT;
                mq_send(pJunqi->search_qid, aBuf, sizeof(SearchMsg), 0);
                nTread++;
            }
            if( !pJunqi->aInfo[(iDir+3)%4].bDead )
            {
                pMsg->type = SEARCH_LEFT;
                mq_send(pJunqi->search_qid, aBuf, sizeof(SearchMsg), 0);
                nTread++;
            }
        }
        while( pJunqi->cntSearch<nTread );//等待所有线程都初始化完毕
        pJunqi->begin_flag = 1;
#endif

        LARGE_INTEGER nBeginTimet;
        LARGE_INTEGER nEndTimet;
        QueryPerformanceCounter(&nBeginTimet);
        pJunqi->test_time[1] = 0;

#ifndef ENG_TEST
        pJunqi->eSearchType = SEARCH_DEFAULT;
        ProSearch(pJunqi,4);
        pJunqi->gFlag[TIME_OUT] = 0;
        pJunqi->begin_time = (unsigned int)time(NULL);

//        MoveSort *pResult[5];
//        FindBestMove(pJunqi,*pJunqi->pEngine->ppMoveSort,pResult,0,3,1);
//        sleep(1);
//        assert(0);

        pJunqi->eSearchType = SEARCH_SINGLE;
        ProSearch(pJunqi,3);
#endif


#ifdef ENG_TEST
        pJunqi->eSearchType = SEARCH_LEFT;
        //pJunqi->eSearchType = SEARCH_RIGHT;
        //pJunqi->eSearchType = SEARCH_SINGLE;
        ProSearch(pJunqi,4);
       // log_a("path %d",CheckDangerPath(pJunqi,3));
        //MakeDeepSearch(pJunqi,pJunqi->eSearchType);

#endif


        QueryPerformanceCounter(&nEndTimet);
        pJunqi->test_time[0] = nEndTimet.QuadPart-nBeginTimet.QuadPart;
        log_a("gen time %d",pJunqi->test_time[1]);
        log_a("gen1 time %d",pJunqi->test_time[0]);


        while(pJunqi->cntSearch);//等所有线程搜索完毕

        SetPathValue(pJunqi);

#ifndef ENG_TEST

    	FindBestPathMove(pJunqi);
#endif
    	ClearMoveSortList(pJunqi);
        log_a("malloc %d",pJunqi->malloc_cnt);
        log_a("free %d",pJunqi->free_cnt);

        assert( pJunqi->malloc_cnt==pJunqi->free_cnt );

//    	memset(pJunqi->pEngine,0,sizeof(Engine));
//    	memset(pJunqi,0,sizeof(Junqi));
//
//     	while(1);
    	//ChecAttackEvent(pEngine);
search_end:

        pJunqi->bSearch = 0;
        pthread_mutex_unlock(&pJunqi->mutex);

    }
    pJunqi->bMove = 0;
    pJunqi->iRpOfst++;
    log_a("iRpOfst %d %d",pJunqi->iRpOfst,pJunqi->nRpStep);

    //if( !pJunqi->bGo || preTurn == pJunqi->eTurn )
    if( preTurn == pJunqi->eTurn )//go指令发了很多条，此时play结果还没发出来时
    {
        return;
    }
    pJunqi->bGo = 0;



	if( pJunqi->eTurn%2==ENGINE_DIR && pJunqi->bAnalyse )
	{
		if( pJunqi->aInfo[pJunqi->eTurn].bDead )
		{
			ChessTurn(pJunqi);
		}

#ifndef EVENT_TEST
		isMove = SendBestMove(pJunqi->pEngine);
#else
		if( pJunqi->nNoEat>8 && value>-100 )
		{
		    isMove = DealEvent(pJunqi->pEngine);
		}
		else
		{
		    isMove = SendBestMove(pJunqi->pEngine);
		}
#endif

		if( !isMove )
		{
			SendRandMove(pJunqi);
		}

	}

}

void *engine_thread(void *arg)
//DWORD WINAPI engine_thread(LPVOID arg)
{
	int len;
	u8 aBuf[REC_LEN];
	Junqi* pJunqi = (Junqi*)arg;

    while (1)
    {
    	len = mq_receive(pJunqi->qid, (char *)aBuf, REC_LEN, NULL);
    	log_a("engine %d\n",len);
        if ( len > 0)
        {
        	//memout(aBuf,len);
        	SafeMemout(aBuf,len);
        	ProRecMsg(pJunqi, aBuf);
        }
    }

	pthread_detach(pthread_self());
	return 0;
}

pthread_t CreatEngineThread(Junqi* pJunqi)
{
    pthread_t tidp;
    mqd_t qid;
    //mq_receive不能小于REC_LEN
    struct mq_attr attr = {0,15,REC_LEN};

  //  struct mq_attr attr1 = {0,20,REC_LEN};

    mq_unlink("engine_msg");
    qid = mq_open("engine_msg", O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
//    if(mq_setattr(qid,&attr1,&attr)<0)
//    {
//        perror("mq_setattr");
//        exit(EXIT_FAILURE);
//    }

    pJunqi->qid = qid;

    mq_unlink("search_msg");
    qid = mq_open("search_msg", O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    pJunqi->search_qid = qid;

//    HANDLE handle1;
//    handle1 = CreateThread(NULL,0,engine_thread,pJunqi,0,NULL);
//    SetThreadAffinityMask(handle1,1);

    pthread_create(&tidp,NULL,(void*)engine_thread,pJunqi);
    return tidp;
}

