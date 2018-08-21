/*
 * engine.c
 *
 *  Created on: Aug 18, 2018
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

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

	assert( pSrc->pLineup->iDir==iDir );

	PlayResult(pJunqi, pSrc, pDst, pResult);
	ChessTurn(pJunqi);
}

BoardChess * GetMoveDst(Junqi* pJunqi, BoardChess *pSrc)
{
	BoardChess *pDst=NULL;
	BoardChess *pTemp;
	u32 rand;
	int i,j;

	rand = random_()%129;
	//rand = 109;
	for(i=0; i<129; i++)
	{
		//if(i==11)
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
    //rand = 21;
    for(i=0;  i<30; i++)
    {
    	assert(pJunqi->eTurn>=0&&pJunqi->eTurn<4);
    	assert((rand+i)%30>=0&&(rand+i)%30<30);
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
	static int isInitBoard = 0;

	if( memcmp(pHead->aMagic, aMagic, 4)!=0 )
	{
		return;
	}

	switch(pHead->eFun)
	{
	case COMM_EVNET:
		event = *((u8*)&pHead[1]);
		ProMoveEvent(pJunqi, pHead->iDir, event);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_MOVE:
		assert( pHead->iDir==pJunqi->eTurn );
		data = (u8*)&pHead[1];
		ProMoveResult(pJunqi, pHead->iDir, data);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_GO:
		pJunqi->bStop = 0;
		break;
	case COMM_START:
		InitChess(pJunqi, data);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		pJunqi->eTurn = pHead->iDir;
		pJunqi->bStart = 1;

		break;
	case COMM_READY:
		pJunqi->bStart = 0;
		SendHeader(pJunqi, pHead->iDir, COMM_READY);
		break;
	case COMM_INIT:
		InitLineup(pJunqi, data, isInitBoard);
		InitChess(pJunqi, data);
		if( !isInitBoard )
		{
			isInitBoard = 1;
			InitBoard(pJunqi);
		}
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_LINEUP:
		data = (u8*)&pHead[1];
		SetRecLineup(pJunqi, data,  pHead->iDir);
		SendHeader(pJunqi, pHead->iDir, COMM_OK);
		break;
	case COMM_STOP:
		pJunqi->bStop = 1;
		break;
	case COMM_ERROR:
		assert(0);
		break;
	default:
		break;
	}

	if( !pJunqi->bStart || pJunqi->bStop )
	{
		return;
	}


	if( pJunqi->eTurn%2==1 )
	{
		sleep(1);
		if( pJunqi->aInfo[pJunqi->eTurn].bDead )
		{
			ChessTurn(pJunqi);
		}
		SendRandMove(pJunqi);
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
    	log_b("engine");
        if ( len > 0)
        {
        	memout(aBuf,len);
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
    struct mq_attr attr = {0,5,REC_LEN};


    mq_unlink("engine_msg");
    qid = mq_open("engine_msg", O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    pJunqi->qid = qid;

    pthread_create(&tidp,NULL,(void*)engine_thread,pJunqi);
    return tidp;
}

