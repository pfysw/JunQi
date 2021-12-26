/*
 * main.c
 *
 *  Created on: Sep 6, 2021
 *      Author: Administrator
 */

#include "junqi.h"
#include "comm.h"


int main(int argc, char *argv[])
{
    Junqi *pJunqi = JunqiOpen();
    pthread_t t2;

    setvbuf(stdout, NULL, _IONBF, 0);
    InitServerIp(argc,argv);

    pthread_create(&t2,NULL,(void*)comm_thread,pJunqi);
    EngineProcess(pJunqi);

    //只能在主线程中创建线程才能多核
    //在其他线程中创建的新线程都与该线程使用同一个cpu

    pthread_join(t2,NULL);

    return 0;
}
