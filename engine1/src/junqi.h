/*
 * junqi.h
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */

#ifndef JUNQI_H_
#define JUNQI_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include "jtype.h"
#include <assert.h>

//#define TEST

#ifdef  TEST
#define ENGINE_DIR   0
#else
#define ENGINE_DIR   1
#endif



enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
                LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};

typedef struct DebugFlag
{
    u8 flagComm;
}DebugFlag;

extern DebugFlag gDebug;
#define log_a(format,...) printf(format"\n",## __VA_ARGS__)
#define log_b(format,...) printf(format,## __VA_ARGS__)

typedef struct BoardChess BoardChess;
typedef struct ChessLineup ChessLineup;

struct ChessLineup
{
    enum ChessType type;
    BoardChess *pPos;
};

struct BoardChess
{

    ChessLineup *pLineup;
};

typedef struct Junqi Junqi;
struct Junqi
{
    BoardChess aChessPos[120];
    ChessLineup aLineup[4][25];
    struct sockaddr_in addr;
    int socket_fd;
    Skeleton *pSkl;
};

Junqi *JunqiOpen(void);
void EngineProcess(Junqi* pJunqi);
void InitLineup(Junqi* pJunqi, u8 *data);

#endif /* JUNQI_H_ */
