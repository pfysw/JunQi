/*
 * junqi.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include <mqueue.h>
#include "skeleton.h"

DebugFlag gDebug = {
        .flagComm = 1
};

Junqi *JunqiOpen(void)
{
    Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
    memset(pJunqi, 0, sizeof(Junqi));
    pJunqi->pSkl = SkeletonOpen(pJunqi);

    return pJunqi;
}

void InitLineup(Junqi* pJunqi, u8 *data)
{
    CommHeader *pHead;
    pHead = (CommHeader *)data;
    int i,j,k;
    int idx;
    ChessLineup *pLineup;
    u8 aCamp[30];

    data = (u8*)&pHead[1];
    k=4;
    memset(aCamp,0,30);

    for(j=0; j<4; j++)
    {
        if( data[j]!=1 ){
            continue;
        }
        idx = 0;
        for(i=0; i<30; i++)
        {
            if(data[k]!=NONE){
                pLineup = &(pJunqi->aLineup[j][idx++]);
                pLineup->type = data[k];
                pJunqi->aChessPos[j*30+i].pLineup = pLineup;
            }
            else{
                aCamp[i] = 1;
            }
            k++;
        }
    }
    for(j=0; j<4; j++)
    {
        idx = 0;
        for(i=0; i<30; i++)
        {
            if( !data[j] && !aCamp[i] )
            {
                pLineup = &(pJunqi->aLineup[j][idx++]);
                pLineup->type = DARK;
                pJunqi->aChessPos[j*30+i].pLineup = pLineup;
            }
        }
    }
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

