/*
 * comm.h
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */

#ifndef COMM_H_
#define COMM_H_
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

#define REC_LEN          1000
const static u8 aMagic[4]={0x57,0x04,0,0};
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
    u8 extra_info;//0~2bit的含义：0：军旗阵亡 1：src是司令 2：dst是司令
    u8 junqi_src[2];
    u8 junqi_dst[2];
}MoveResultData;

void *comm_thread(void *arg);
void InitServerIp(int argc, char *argv[]);

#endif /* COMM_H_ */
