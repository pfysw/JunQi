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
#define log_a(format,...)   SafePrint(format"\n",## __VA_ARGS__)
#define log_fun(format,...)  //printf(format"\n",## __VA_ARGS__)
#define log_b(format,...)   SafePrint(format"\n",## __VA_ARGS__)
#define log_c(format,...)   printf(format"\n",## __VA_ARGS__)

void memout(u8 *pdata,int len);

void SafePrint(const char *zFormat, ...);
void SafeMemout(u8 *aBuf,int len);
pthread_t CreatPrintThread(Junqi* pJunqi);

#endif /* UTILIY_H_ */
