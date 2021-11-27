/*
 * junqi.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include <mqueue.h>

Junqi *JunqiOpen(void)
{
    Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
    memset(pJunqi, 0, sizeof(Junqi));

    return pJunqi;
}

mqd_t CreateMessageQueue(char *name,int len)
{
    mqd_t qid;
    struct mq_attr attr = {0,15,len};
    mq_unlink(name);
    qid = mq_open(name, O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    return qid;
}

void EngineProcess(Junqi* pJunqi)
{
    printf("engine\n");
    while(1){

    }
}

/* delete[1]
void *engine_thread(void *arg)
{
    printf("engine thread\n");
    while(1){

    }
    pthread_detach(pthread_self());
    return NULL;
}

pthread_t CreatEngineThread(Junqi* pJunqi)
{
    pthread_t tidp;

    pthread_create(&tidp,NULL,(void*)engine_thread,pJunqi);
    return tidp;
}
*/
