/*
 * comm.h
 *
 *  Created on: Aug 16, 2018
 *      Author: Administrator
 */

#ifndef COMM_H_
#define COMM_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pthread.h"
#include "type.h"
#include <mqueue.h>

#define COMM_OK          0
#define COMM_ERROR       1
#define COMM_GO          2
#define COMM_MOVE        3
#define COMM_EVNET       4
#define COMM_START       5
#define COMM_READY       6
#define COMM_LINEUP      7
#define COMM_INIT        8
#define COMM_STOP        9


#define REC_LEN          200
const static u8 aMagic[4]={0x57,0x04,0,0};

typedef struct CommHeader
{
	u8 aMagic[4];
	u8 iDir;
	u8 eFun;
	u8 reserve[2];
}CommHeader;

typedef struct MoveResultData
{
	u8 src[2];
	u8 dst[2];
	u8 result;
	u8 extra_info;
	u8 junqi_src[2];
	u8 junqi_dst[2];
}MoveResultData;

pthread_t CreatCommThread(Junqi* pJunqi);
void SendHeader(Junqi* pJunqi, u8 iDir, u8 eFun);
void SendMove(Junqi* pJunqi, BoardChess *pSrc, BoardChess *pDst);
void SendEvent(Junqi* pJunqi, int iDir, u8 event);
void SetRecLineup(Junqi* pJunqi, u8 *data, int iDir);

#endif /* COMM_H_ */
