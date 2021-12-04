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

//#define TEST

#ifdef  TEST
#define ENGINE_DIR   0
#else
#define ENGINE_DIR   1
#endif

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

typedef struct DebugFlag
{
    u8 flagComm;
}DebugFlag;

extern DebugFlag gDebug;
#define log_a(format,...) printf(format"\n",## __VA_ARGS__)
#define log_b(format,...) printf(format,## __VA_ARGS__)

typedef struct Junqi Junqi;
struct Junqi
{
    struct sockaddr_in addr;
    int socket_fd;
};

Junqi *JunqiOpen(void);
//void *engine_thread(void *arg);//delete[1]
void EngineProcess(Junqi* pJunqi);

#endif /* JUNQI_H_ */
