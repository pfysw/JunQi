/*
 * utiliy.h
 *
 *  Created on: Sep 22, 2018
 *      Author: Administrator
 */

#ifndef UTILIY_H_
#define UTILIY_H_
#include "type.h"
#include <stdarg.h>
#include <mqueue.h>
#include <unistd.h>
#include "pthread.h"

////////// test /////////////////////
#define log_a(format,...)  SafePrint(format"\n",## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)
//用作子力概率分析
#define log_b(format,...)   //SafePrint(format"\n",## __VA_ARGS__)
//#define log_b(format,...)   printf(format"\n",## __VA_ARGS__)
#define log_c(format,...)   printf(format"\n",## __VA_ARGS__)

void memout(u8 *pdata,int len);

void SafePrint(const char *zFormat, ...);
void SafeMemout(u8 *aBuf,int len);
pthread_t CreatPrintThread(Junqi* pJunqi);

int memsys5Init(Junqi *pJunqi, int nHeap, int mnReq);
void *memsys5Malloc(Junqi *pJunqi, int nByte);
void memsys5Free(Junqi *pJunqi, void *pOld);

MoveSort *SortMoveValueList(MoveSort *pIn, int type, int depth);

#endif /* UTILIY_H_ */
