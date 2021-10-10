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
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

typedef struct Junqi Junqi;
struct Junqi
{

};

Junqi *JunqiOpen(void);
void *engine_thread(void *arg);

#endif /* JUNQI_H_ */
