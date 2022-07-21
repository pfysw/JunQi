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
#define COMM_REPLAY      10
#define COMM_VERIFY      11
#define COMM_PATH_DEBUG  12


#define LOCAL_PORT  1234
#define DST_PORT    5678

#define REC_LEN          1000
#define MAX_PAIR 200

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
    // 0~3bit的含义：0：军旗阵亡 1：src是司令 2：dst是司令 3：无棋可走
	u8 extra_info;
	u8 junqi_src[2];
	u8 junqi_dst[2];
}MoveResultData;

typedef struct ChessTypeD
{
    u8 color;
//    u8 parent;
    u8 num;
}ChessTypeD;

typedef struct PathPair
{
    u8 src;
    u8 dst;
}PathPair;

typedef struct PathDebug
{
    ChessTypeD aChess[129];
    int nPair;
    PathPair aDir[MAX_PAIR];
}PathDebug;

void CreatCommThread(Junqi* pJunqi);
void SendHeader(Junqi* pJunqi, u8 iDir, u8 eFun);
void SendLineup(Junqi* pJunqi, int iDir);
void SendMoveResult(Junqi* pJunqi, int iDir, MoveResultData *pData);
void SendEvent(Junqi* pJunqi, int iDir, u8 event);
void SendReplyToEngine(Junqi *pJunqi);
void SendVerifyMsg(Junqi* pJunqi);
int CalDecNum();
void SendProxy(Junqi* pJunqi);

#endif /* COMM_H_ */
