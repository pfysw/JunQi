/*
 * comm.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */
#include "junqi.h"

void *comm_thread(void *arg)
{
    printf("comm thread\n");
    while(1){

    }
    pthread_detach(pthread_self());
    return NULL;
}

pthread_t CreatCommThread(Junqi* pJunqi)
{
    pthread_t tidp;
    pthread_create(&tidp,NULL,(void*)comm_thread,pJunqi);
    return tidp;
}
