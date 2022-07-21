/*
 * communication.c
 *
 *  Created on: Aug 15, 2018
 *      Author: Administrator
 */

#include "comm.h"
#include "pthread.h"
#include "junqi.h"
#include "rule.h"
#include "board.h"

int aVerify[2] = {
        4011111,
        3922222
};
int open_num = 888;
char server_ip[30] = "172.0.0.1";

gboolean pro_comm_msg(gpointer data);

int CalDecNum(){
    int result = 0;
    result = open_num*aVerify[0]%1000;
    result = result*aVerify[1]%1000;
    return result;
}

void PacketHeader(CommHeader *header, u8 iDir, u8 eFun)
{
	memset(header, 0, sizeof(CommHeader));
	memcpy(header->aMagic, aMagic, 4);
	//目前该位只在COMM_START标识先手位
	header->iDir = iDir;
	header->eFun = eFun;
}

void SendData(Junqi* pJunqi, CommHeader *header, void *data, int len)
{
	u8 buf[200];
	int length = 0;

	length += sizeof(CommHeader);
	memcpy(buf, header, length);

	memcpy(buf+length, data, len);
	length += len;
	//在复盘中不可以向引擎发送行棋结果
	//但是需要分析的时候可以
    if( !pJunqi->bReplay || pJunqi->bAnalyse )
    {
        log_b("send fun %d len %d port %d",buf[5],length,pJunqi->addr.sin_port);
		sendto(pJunqi->socket_fd, buf, length, 0,
				(struct sockaddr *)&pJunqi->addr, sizeof(struct sockaddr));
    }
}

void SendReplyData(Junqi* pJunqi, CommHeader *header, void *data, int len)
{
	u8 buf[4096];
	int length = 0;

	length += sizeof(CommHeader);
	memcpy(buf, header, length);

	memcpy(buf+length, data, len);
	length += len;

	//log_b("send fun %d len %d port %d",buf[5],length,pJunqi->addr.sin_port);
	sendto(pJunqi->socket_fd, buf, length, 0,
			(struct sockaddr *)&pJunqi->addr, sizeof(struct sockaddr));

}

void SendHeader(Junqi* pJunqi, u8 iDir, u8 eFun)
{
	CommHeader header;

	if( eFun!=COMM_START && eFun!=COMM_REPLAY )
	{
		pJunqi->addr = pJunqi->addr_tmp[(iDir+1)%2];
	}
	PacketHeader(&header, iDir, eFun);
	SendData(pJunqi, &header, NULL, 0);
}

void SendProxy(Junqi* pJunqi)
{
    struct sockaddr_in addr;
    char buf[100] = "register！";

    addr.sin_family = AF_INET;
    inet_pton(AF_INET, "42.192.193.83", &addr.sin_addr);
   // inet_pton(AF_INET, "115.194.188.23", &addr.sin_addr);
    addr.sin_port = htons(5555);
    sendto(pJunqi->socket_fd, buf, strlen(buf), 0,
            (struct sockaddr *)&addr, sizeof(struct sockaddr));
}

void GetSendLineup(Junqi* pJunqi, u8 *data, int iDir)
{
	int i;
	for(i=0; i<30; i++)
	{
		data[i] = pJunqi->Lineup[iDir][i].type;
	}

}

void SendLineup(Junqi* pJunqi, int iDir)
{
	CommHeader header;
	u8 aData[30];
#ifdef NOT_DEBUG2
	if( iDir%2!=0 )
#else
	//pthread_mutex_lock(&pJunqi->mutex);//这里为什么要加？
	if( iDir%2!=0 )
	{
		//pJunqi->socket_fd = pJunqi->socket_tmp[0];
		pJunqi->addr = pJunqi->addr_tmp[0];
	}
	else
	{
		//pJunqi->socket_fd = pJunqi->socket_tmp[1];
		pJunqi->addr = pJunqi->addr_tmp[1];
	}
	//pthread_mutex_unlock(&pJunqi->mutex);
#endif
	{
		PacketHeader(&header, iDir, COMM_LINEUP);
		GetSendLineup(pJunqi, aData, iDir);
		SendData(pJunqi, &header, aData, 30);
	}
}

void SendMoveResult(Junqi* pJunqi, int iDir, MoveResultData *pData)
{
	CommHeader header;
	PacketHeader(&header, iDir, COMM_MOVE);
	SendData(pJunqi, &header, pData, sizeof(MoveResultData));
}

void SendVerifyMsg(Junqi* pJunqi)
{
    CommHeader header;
    int aNum[2];
    pJunqi->addr = pJunqi->addr_tmp[0];
    PacketHeader(&header, 1, COMM_VERIFY);
    aNum[0] = 11111;
    aNum[1] = 22222;
    printf("verify addr %d\n",pJunqi->addr.sin_port);
    SendData(pJunqi, &header, aNum, sizeof(aNum));
    log_b("verify end");
}

void SendReplyToEngine(Junqi *pJunqi)
{

	CommHeader header;
	int len;
	PacketHeader(&header, pJunqi->eFirstTurn, COMM_REPLAY);
	memcpy(header.reserve,&pJunqi->iRpStep,2);
	//len = (*((int *)(&pJunqi->aReplay[4])))*4+128;
	len = 128;

    pJunqi->addr = pJunqi->addr_tmp[0];
    SendReplyData(pJunqi, &header, pJunqi->aReplay,len);
#ifndef NOT_DEBUG2
    pJunqi->addr = pJunqi->addr_tmp[1];
    SendReplyData(pJunqi, &header, pJunqi->aReplay,len);
#endif

}

void SendEvent(Junqi* pJunqi, int iDir, u8 event)
{
	CommHeader header;
	u8 data[4] = {0};
	data[0] = event;
	data[1] = pJunqi->aInfo[iDir].cntJump;
	PacketHeader(&header, iDir, COMM_EVNET);
	SendData(pJunqi, &header, &data, 4);
}

void DealRecData(Junqi* pJunqi, u8 *data)
{
	CommHeader *pHead;
	pHead = (CommHeader *)data;
	BoardChess *pSrc, *pDst;
	BoardPoint p1,p2;
	u8 event;
	//log_b("line: %d",pHead->iDir);

	if( memcmp(pHead->aMagic, aMagic, 4)!=0 )
	{
		return;
	}

	//复盘的时候引擎发过来的行棋不要处理
#ifndef  SIMULATION
	if( pJunqi->bAnalyse || pJunqi->bReplay )
	{
		if( pHead->eFun==COMM_MOVE || pHead->eFun==COMM_EVNET )
		{
			return;
		}
	}
#endif


	log_b("fun %d",pHead->eFun);
	switch(pHead->eFun)
	{
    case COMM_VERIFY:
        log_a("verify %d %d",*((int*)&data[8]),*((int*)&data[12]));
        if(aVerify[0]==*((int*)&data[8]) &&
           aVerify[1]==*((int*)&data[12]) ){
            //gtk_widget_show(pJunqi->apLabel[0]);
            ShowLabel(pJunqi->apLabel[0],"测试中","FF34B3",24);
            pJunqi->aTestFlag[0] = 1;
        }
        else{
            ShowLabel(pJunqi->apLabel[0],"验证失败","FF34B3",24);
            pJunqi->aTestFlag[0] = 0;
        }
        break;
	case COMM_READY:
		//PacketHeader(pHead, 0, COMM_INIT);
		pHead->eFun = COMM_INIT;
		data = (u8*)&pHead[1];
		memset(data, 0, 64);//前4个字节表示要发送哪一家布阵
		if( pHead->iDir%2==1 )
		{
			//log_b("line 0");
			data[1] = data[3] = 1;
			GetSendLineup(pJunqi, data+4, 1);
			GetSendLineup(pJunqi, data+34, 3);
		}
		else
		{
		//	log_b("line 1");
			data[0] = data[2] = 1;
			GetSendLineup(pJunqi, data+4, 0);
			GetSendLineup(pJunqi, data+34, 2);
		}
		SendData(pJunqi, pHead, data, 64);
		printf("ready state %d\n",pJunqi->eState);
		pJunqi->eState = 1;
#ifdef AUTO_TEST
		//log_b("bAutoTest %d",pJunqi->bAutoTest);
		if( pJunqi->bAutoTest==1 ||
		        pJunqi->bAutoTest==2 )
		{
		    pJunqi->bAutoTest++;
		}
#endif
		break;
	case COMM_MOVE:
		data = (u8*)&pHead[1];
		p1.x = data[0]%17;
		p1.y = data[1]%17;
		p2.x = data[2]%17;
		p2.y = data[3]%17;
		if( pJunqi->bStart && pJunqi->eTurn==pHead->iDir
#ifdef NOT_DEBUG2
				&& pHead->iDir%2==1
#endif
				)
		{
			if( pJunqi->aBoard[p1.x][p1.y].pAdjList && pJunqi->aBoard[p2.x][p2.y].pAdjList )
			{
				pSrc = pJunqi->aBoard[p1.x][p1.y].pAdjList->pChess;
				pDst = pJunqi->aBoard[p2.x][p2.y].pAdjList->pChess;
				if( pSrc==NULL || pDst==NULL || pSrc->type==NONE )
				{
					SendHeader(pJunqi, pHead->iDir, COMM_ERROR);
					return;
				}
				if( pDst->type!=NONE && pSrc->pLineup->iDir%2==pDst->pLineup->iDir%2 )
				{
					SendHeader(pJunqi, pHead->iDir, COMM_ERROR);
					return;
				}
			}
			else
			{
				SendHeader(pJunqi, pHead->iDir, COMM_ERROR);
				return;
			}

			if( pSrc->pLineup->iDir==pHead->iDir && IsEnableMove(pJunqi, pSrc, pDst, 1) )
			{
				gtk_widget_hide(pJunqi->redRectangle[0]);
				gtk_widget_hide(pJunqi->redRectangle[1]);
			    gtk_widget_hide(pJunqi->whiteRectangle[0]);
			    gtk_widget_hide(pJunqi->whiteRectangle[1]);

				int type;
				type = CompareChess(pSrc, pDst);
				PlayResult(pJunqi, pSrc, pDst, type);
				ChessTurn(pJunqi);
			}
			else
			{
				SendHeader(pJunqi, pHead->iDir, COMM_ERROR);
			}
		}
#ifdef AUTO_TEST
                AutoTest(pJunqi);
#endif
		break;
	case COMM_EVNET:
		if( pJunqi->bStart && pJunqi->eTurn==pHead->iDir
#ifdef NOT_DEBUG2
				&& pHead->iDir%2==1
#endif
				)
		{
			event = *((u8*)&pHead[1]);
			if( event==JUMP_EVENT )
			{
				log_b("jump");
				AddEventToReplay(pJunqi, JUMP_EVENT, pHead->iDir);
				IncJumpCnt(pJunqi, pHead->iDir);
				ChessTurn(pJunqi);

				pJunqi->addr = pJunqi->addr_tmp[0];
				SendEvent(pJunqi, pHead->iDir, JUMP_EVENT);
#ifndef NOT_DEBUG2
				pJunqi->addr = pJunqi->addr_tmp[1];
				SendEvent(pJunqi, pHead->iDir, JUMP_EVENT);
#endif

			}
			else if( event==SURRENDER_EVENT )
			{
				pJunqi->addr = pJunqi->addr_tmp[0];
				SendEvent(pJunqi, pHead->iDir, SURRENDER_EVENT);
#ifndef NOT_DEBUG2
				pJunqi->addr = pJunqi->addr_tmp[1];
				SendEvent(pJunqi, pHead->iDir, SURRENDER_EVENT);
#endif

				SendSoundEvent(pJunqi,DEAD);
				DestroyAllChess(pJunqi, pHead->iDir);
				ClearChessFlag(pJunqi,pHead->iDir);
				AddEventToReplay(pJunqi, SURRENDER_EVENT, pHead->iDir);
				ChessTurn(pJunqi);
			}
			else
			{
				SendHeader(pJunqi, pHead->iDir, COMM_ERROR);
			}
		}
#ifdef AUTO_TEST
                AutoTest(pJunqi);
#endif
		break;
	case COMM_LINEUP:
		if( pHead->iDir%2==1 )
		{
			LoadLineup(pJunqi, pHead->iDir, (u8*)&pHead[1]);
		}
		break;
	case COMM_REPLAY:
		break;
	case COMM_OK:
	    printf("ok state %d\n",pJunqi->eState);
	    if(pJunqi->eState==1){
	        pJunqi->eState = 2;
	    }
		if( pJunqi->bAnalyse && !pJunqi->bReplay )
		{
#ifdef SIMULATION
		    pJunqi->eReply++;
		    if(pJunqi->eReply==2){
#endif
                //SendHeader(pJunqi, pJunqi->eFirstTurn, COMM_REPLAY);
                SendReplyToEngine(pJunqi);
                ShowReplayStep(pJunqi, 0);
                //下一次进入ShowReplayStep再把bAnalyse清0
                //因为分析该局面后可能还会再手动推演
                pJunqi->bAnalyse = 1;
                pJunqi->bReplay = 1;
#ifdef SIMULATION
                pJunqi->eReply = 0;
		    }
#endif
		}
#ifdef AUTO_TEST
		if(pJunqi->bAutoTest==3)
		{
		    sleep(1);
		    pJunqi->bAutoTest = 0;
		    begin_button(pJunqi->begin_button,NULL,pJunqi);

		}
#endif
		break;
	case COMM_PATH_DEBUG:
#if PATH_DEBUG
	    printf("debug\n");
	    data = (u8*)&pHead[1];
	    ShowDebugPath(pJunqi,(PathDebug *)data);
#endif
	    break;
	default:
		break;
	}

}


void *comm_thread(void *arg)
{
	Junqi* pJunqi = (Junqi*)arg;
	int socket_fd;
	struct sockaddr_in addr,local;
	struct sockaddr_in addr1;
	struct sockaddr_in rec_addr;
	int from_len;
	int recvbytes = 0;
	u8 buf[REC_LEN]={0};
	u8 tmp_buf[REC_LEN]={0};

	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		printf("Create Socket Failed!\n");
		pthread_detach(pthread_self());
	}

	local.sin_family = AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;
	local.sin_port = htons(1234);
	if(bind(socket_fd, (struct sockaddr *)&local, sizeof(struct sockaddr) )<0)
	{
		printf("Bind Error!\n");
		pthread_detach(pthread_self());
	}

	addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(DST_PORT);
	memcpy(&addr1, &addr, sizeof(addr));
	addr1.sin_port = htons(6678);


	pJunqi->socket_fd = socket_fd;

	pJunqi->addr_tmp[0] = addr;
	pJunqi->addr_tmp[1] = addr1;
	SendHeader(pJunqi, 0, COMM_READY);
	SendHeader(pJunqi, 1, COMM_READY);
	SendProxy(pJunqi);
	from_len = sizeof(struct sockaddr_in);

//    int result = CalDecNum();
//    printf("%d\n",result);
	while(1)
	{
        //recvbytes=recvfrom(socket_fd, buf, REC_LEN, 0,NULL,NULL);
		recvbytes=recvfrom(socket_fd, buf, REC_LEN, 0,
		        (struct sockaddr *)&rec_addr,&from_len);
		printf("rec ip %s port %d %d\n",inet_ntoa(rec_addr.sin_addr),
		        htons(rec_addr.sin_port),rec_addr.sin_port);
		//log_b("len %d",recvbytes);
		pthread_mutex_lock(&pJunqi->mutex);
		pJunqi->addr = rec_addr;
		memcpy(tmp_buf,buf,recvbytes);
		pJunqi->pCommData = tmp_buf;
		//pthread_mutex_unlock(&pJunqi->mutex);
		g_idle_add((GSourceFunc)pro_comm_msg, pJunqi);

	}

	pthread_detach(pthread_self());
	return NULL;

}

gboolean pro_comm_msg(gpointer data)
{
	Junqi *pJunqi = (Junqi *)data;

	//pthread_mutex_lock(&pJunqi->mutex);
#ifdef NOT_DEBUG2
	DealRecData(pJunqi, pJunqi->pCommData);
#else
	CommHeader *pHead;
	pHead = (CommHeader *)pJunqi->pCommData;

	if(!pJunqi->bStart)
	{
        if( pHead->iDir%2==1 )
        {
            //log_b("dir0 %d",pHead->iDir);
           // pJunqi->addr = pJunqi->addr_tmp[0];
            pJunqi->addr_tmp[0] = pJunqi->addr;
        }
        else
        {
            //log_b("dir1 %d",pHead->iDir);
           // pJunqi->addr = pJunqi->addr_tmp[1];
            pJunqi->addr_tmp[1] = pJunqi->addr;
        }
        log_b("dir %d",pHead->iDir);
	}
	DealRecData(pJunqi, pJunqi->pCommData);


#endif
	pthread_mutex_unlock(&pJunqi->mutex);

	return 0;
}

void CreatCommThread(Junqi* pJunqi)
{
    pthread_t tidp;
    pthread_create(&tidp,NULL,(void*)comm_thread,pJunqi);

//    pthread_t tidp1;
//    pthread_create(&tidp1,NULL,(void*)comm_thread1,pJunqi);
}

