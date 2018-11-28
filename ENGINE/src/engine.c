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

int preTurn = 1000;

EventHandle eventArr[] = {
	{ ComeInCamp, CAMP_EVENT },
	{ ProBombEvent, BOMB_EVENT },
	{ ProEatEvent, EAT_EVENT },
	{ ProEatEvent, GONGB_EVENT },
	{ ProEatEvent, DARK_EVENT },
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

void CheckMoveEvent(
	Engine *pEngine,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData* pResult)
{
	int type = pResult->result;
	if( type==MOVE || type==EAT  )
	{
		if( pDst->pLineup->iDir%2!=ENGINE_DIR%2 )
		{
			CheckCampEvent(pEngine,pDst);
		}
	}
	CheckBombEvent(pEngine);
	CheckEatEvent(pEngine);
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
		    CheckMoveEvent(pJunqi->pEngine, pSrc, pDst, pResult);
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
    	log_a("i %d rand %d k %d",i, rand,(rand+i)%30);
    	log_a("type %d",pLineup->type);
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
    for(int i=0; i<30; i++)
    {
        pEngine->aBestMove[i].flag1 = 0;
        pEngine->aBestMove[i].mxPerFlag = 0;
        pEngine->aBestMove[i].mxPerFlag1 = 0;
    }
}

void ProRecMsg(Junqi* pJunqi, u8 *data)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	u8 event;
	u8 isMove = 0;
	int value;
	int eTurn;
	Engine *pEngine = pJunqi->pEngine;
	int i;
	int analyseFlag = 0;

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

	if( !pJunqi->bStart || pJunqi->bStop )
	{
		return;
	}
    if( 0==pJunqi->nRpStep ||
    	pJunqi->iRpOfst>pJunqi->nRpStep-1 )
    {
        analyseFlag = 1;
		//value = EvalSituation(pJunqi);
    	eTurn = pJunqi->eTurn;
    	log_b("search1");
//		pJunqi->eTurn = eTurn;
//		value = AlphaBeta(pJunqi,4,-INFINITY,INFINITY);
//		log_b("depth %d value %d",4,value);
    	pJunqi->bGo = 0;
    	pJunqi->bMove = 0;
    	pJunqi->begin_time = (unsigned int)time(NULL);

    	memset(pEngine->aBestMove,0,sizeof(pEngine->aBestMove));

    	for(i=0; i<5; i++)
    	{
    		pJunqi->eTurn = eTurn;
    		pthread_mutex_lock(&pJunqi->mutex);
    		pJunqi->bSearch = 1;
    		pJunqi->test_num = 0;
    		pJunqi->test_gen_num = 0;
    		pJunqi->searche_num[0] = 0;
    		pJunqi->searche_num[1] = 0;
    		ClearBestMoveFlag(pEngine);

            LARGE_INTEGER nBeginTime;
            LARGE_INTEGER nEndTime;
            QueryPerformanceCounter(&nBeginTime);
    		pJunqi->test_time[1] = 0;

			//value = AlphaBeta(pJunqi,i,-INFINITY,INFINITY);
			value = AlphaBeta1(pJunqi,i,-INFINITY,INFINITY);
    		//value = AlphaBetaTest(pJunqi,i,-INFINITY,INFINITY);
    		//value = AlphaBetaTest(pJunqi,i,4,5);
			log_a("search1 num %d",pJunqi->test_num);
			log_a("gen num %d",pJunqi->test_gen_num);
			log_a("key num %d %d",pJunqi->searche_num[0],
			        pJunqi->searche_num[1]);
			pJunqi->bSearch = 0;
			pthread_mutex_unlock(&pJunqi->mutex);
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

		    QueryPerformanceCounter(&nEndTime);
		    pJunqi->test_time[0] = nEndTime.QuadPart-nBeginTime.QuadPart;
			log_a("gen time %d",pJunqi->test_time[1]);
			log_a("gen0 time %d",pJunqi->test_time[0]);

			if( TimeOut(pJunqi) )
			{
				log_a("break");
				break;
			}
			if( eTurn%2!=ENGINE_DIR%2 )
			{
				value = -value;
			}

			log_a("depth %d value %d",i,value);
    	}

    	FreeBestMoveList(pEngine->aBestMove,i);
    	//GenerateMoveList1(pJunqi,eTurn);
//    	SafeMemout(pJunqi->aInfo[3].aLiveTypeSum,14);
//    	SafeMemout(pJunqi->aInfo[3].aLiveAllNum,14);

    	pJunqi->eTurn = eTurn;

    }
    pJunqi->bMove = 0;
    pJunqi->iRpOfst++;

    //if( !pJunqi->bGo || preTurn == pJunqi->eTurn )
    if( preTurn == pJunqi->eTurn )
    {
        return;
    }
    pJunqi->bGo = 0;

	if( pJunqi->eTurn%2==ENGINE_DIR && analyseFlag )
	{
		if( pJunqi->aInfo[pJunqi->eTurn].bDead )
		{
			ChessTurn(pJunqi);
		}

#ifndef EVENT_TEST
		isMove = SendBestMove(pJunqi->pEngine);
#else
		if( pJunqi->nNoEat>8 )//&& value>-100 )
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
	return NULL;
}

pthread_t CreatEngineThread(Junqi* pJunqi)
{
    pthread_t tidp;
    mqd_t qid;
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

    pthread_create(&tidp,NULL,(void*)engine_thread,pJunqi);
    return tidp;
}

