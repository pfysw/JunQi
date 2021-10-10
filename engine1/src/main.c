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
    pthread_t t1,t2;

    setvbuf(stdout, NULL, _IONBF, 0);

    pthread_create(&t1,NULL,(void*)engine_thread,pJunqi);
    pthread_create(&t2,NULL,(void*)comm_thread,pJunqi);

    //ֻ�������߳��д����̲߳��ܶ��
    //�������߳��д��������̶߳�����߳�ʹ��ͬһ��cpu

    pthread_join(t1,NULL);

    return 0;
}
