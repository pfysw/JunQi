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
#include "type.h"

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


#define LOCAL_PORT  1234
#define DST_PORT    5678

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

void CreatCommThread(Junqi* pJunqi);
void SendHeader(Junqi* pJunqi, u8 iDir, u8 eFun);
void SendLineup(Junqi* pJunqi, int iDir);
void SendMoveResult(Junqi* pJunqi, int iDir, MoveResultData *pData);
void SendEvent(Junqi* pJunqi, int iDir, u8 event);

#endif /* COMM_H_ */
