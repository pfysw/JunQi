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
			return;
		}
	}
	else
	{
		SendHeader(pJunqi, iDir, COMM_ERROR);
		return;
	}
	//log_c("dir %d %d\n",pSrc->pLineup->iDir,iDir);
	assert( pSrc->pLineup->iDir==iDir );

	//test
//#define POS_TEST
#ifdef POS_TEST

	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	PartyInfo aInfo[4];
	int dir;
	memcpy(Lineup, pJunqi->Lineup,sizeof(Lineup));
	memcpy(ChessPos, pJunqi->ChessPos,sizeof(ChessPos));
	memcpy(aInfo, pJunqi->aInfo,sizeof(aInfo));
	if( pJunqi->iRpOfst==307 )
	{
		log_c("debug");
	}
	PushMoveToStack(pJunqi, pSrc, pDst, pResult);
	assert( !memcmp(Lineup, pJunqi->Lineup,sizeof(Lineup)) );
#endif
	PlayResult(pJunqi, pSrc, pDst, pResult);
#ifdef POS_TEST
	//assert( !memcmp(Lineup, pJunqi->Lineup,sizeof(Lineup)) );
	PopMoveFromStack(pJunqi, pSrc, pDst, pResult);
//	 log_c("----------");
//    memout(pJunqi->ChessPos[pSrc->pLineup->iDir], sizeof(pJunqi->ChessPos[0]));
//    log_c("+++++++++++");
//    memout(ChessPos[pSrc->pLineup->iDir], sizeof(pJunqi->ChessPos[0]));
//    log_c("----------");
//	if( pResult->result!=MOVE )
//	{
//		memout(pJunqi->ChessPos[pDst->pLineup->iDir], sizeof(pJunqi->ChessPos[0]));
//		 log_c("+++++++++++");
//		 memout(pJunqi->ChessPos[pDst->pLineup->iDir], sizeof(pJunqi->ChessPos[0]));
//	}
	assert( !memcmp(Lineup, pJunqi->Lineup,sizeof(Lineup)) );
	assert( !memcmp(ChessPos, pJunqi->ChessPos,sizeof(ChessPos)) );
	if( pResult->result!=MOVE )
	{
		if( pSrc->pLineup->iDir%2==ENGINE_DIR%2 )
		{
			dir = pDst->pLineup->iDir;
		}
		else
		{
			dir = pSrc->pLineup->iDir;
		}
		assert( !memcmp(&aInfo[dir], &pJunqi->aInfo[dir],sizeof(PartyInfo)) );
	}

	assert( aInfo[ENGINE_DIR].bDead == pJunqi->aInfo[ENGINE_DIR].bDead );
	PlayResult(pJunqi, pSrc, pDst, pResult);
#endif
	//-----------------
	if( pJunqi->bStart )
	{
		ChessTurn(pJunqi);
		//CheckMoveEvent(pJunqi->pEngine, pSrc, pDst, pResult);
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


void ProRecMsg(Junqi* pJunqi, u8 *data)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	u8 event;
	u8 isMove = 0;
	int value;
	int eTurn;

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
		//log_c("turn %d %d",pHead->iDir,pJunqi->eTurn);
		assert( pHead->iDir==pJunqi->eTurn );
		data = (u8*)&pHead[1];

		ProMoveResult(pJunqi, pHead->iDir, data);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_REPLAY:
		log_b("reply %d",pHead->iDir);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		pJunqi->eTurn = pHead->iDir;
		pJunqi->bStart = 1;
		//在COMM_START指令中清0
		pJunqi->nRpStep = *((u16*)pHead->reserve);
		pJunqi->iRpOfst = 0;

		//获取复盘布阵，以后可能有用
//		data = (u8*)&pHead[1];
//		InitReplyLineup(pJunqi,&data[8]);

		//没有必要再往下执行，等待接收棋谱
		return;
		break;
	default:
		break;
	}

	if( !pJunqi->bStart || pJunqi->bStop )
	{
		return;
	}
    if( 0==pJunqi->nRpStep ||
    	pJunqi->iRpOfst==pJunqi->nRpStep-1 )
    {
		//value = EvalSituation(pJunqi);
    	eTurn = pJunqi->eTurn;
    	log_b("search");
//		pJunqi->eTurn = eTurn;
//		value = AlphaBeta(pJunqi,4,-INFINITY,INFINITY);
//		log_b("depth %d value %d",4,value);
    	pJunqi->bGo = 0;
    	pJunqi->bMove = 0;
    	for(int i=1; ;i++)
    	{
    		pJunqi->eTurn = eTurn;
    		pthread_mutex_lock(&pJunqi->mutex);
    		pJunqi->bSearch = 1;
    		pJunqi->test_num = 0;
			value = AlphaBeta(pJunqi,i,-INFINITY,INFINITY);
			log_b("search num %d",pJunqi->test_num);
			pJunqi->bSearch = 0;
			pthread_mutex_unlock(&pJunqi->mutex);
			if( TimeOut(pJunqi) )
			{
				log_b("break");
				break;
			}
			if( eTurn%2!=ENGINE_DIR%2 )
			{
				value = -value;
			}
			log_b("depth %d value %d",i,value);
    	}
    	pJunqi->eTurn = eTurn;

    }
    pJunqi->bMove = 0;
    pJunqi->iRpOfst++;

	if( !pJunqi->bGo || preTurn == pJunqi->eTurn )
	{
		return;
	}
	pJunqi->bGo = 0;

	if( pJunqi->eTurn%2==ENGINE_DIR )
	{
		if( pJunqi->aInfo[pJunqi->eTurn].bDead )
		{
			ChessTurn(pJunqi);
		}

		//isMove = DealEvent(pJunqi->pEngine);
		isMove = SendBestMove(pJunqi->pEngine);

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
    	log_b("engine %d\n",len);
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

    struct mq_attr attr1 = {0,20,REC_LEN};

    mq_unlink("engine_msg");
    qid = mq_open("engine_msg", O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    if(mq_setattr(qid,&attr1,&attr)<0)
    {
        perror("mq_setattr");
        exit(EXIT_FAILURE);
    }

    pJunqi->qid = qid;

    pthread_create(&tidp,NULL,(void*)engine_thread,pJunqi);
    return tidp;
}

