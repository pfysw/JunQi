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

void InitLineup(Junqi* pJunqi, u8 *data)
{
    CommHeader *pHead;
    pHead = (CommHeader *)data;
    int i,j,k;
    int idx;
    ChessLineup *pLineup;
    BoardChess *pChess;
    int nChess = 30;

    data = (u8*)&pHead[1];
    k=4;
    for(j=0; j<4; j++)
    {
        idx = 0;
        for(i=0; i<30; i++)
        {
            pChess = &(pJunqi->aChessPos[j*nChess+i]);
            if(pChess->prop!=CAMP){
                pLineup = &(pJunqi->aLineup[j][idx++]);
                if(data[j]) {
                    pLineup->type = data[k++];
                }
                else{
                    pLineup->type = DARK;
                }
                pChess->pLineup = pLineup;
            }
            else if(data[j]){
                k++;
            }
        }
    }
}

void SetChessPosProperty(Junqi* pJunqi)
{
    int i,j;
    BoardChess *pChess;
    int nChess = 30;

    for(j=0;j<4;j++)
    {
        for(i=0;i<nChess;i++)
        {
            pChess = &(pJunqi->aChessPos[j*nChess+i]);

            if(i==6||i==8||i==12||i==16||i==18){
                pChess->prop = CAMP;
            }
            else if(i<25)
            {
                if( (i/5==0||i/5==4) || ((i%5==0||i%5==4)) )
                {
                    pChess->prop = RAILWAY;
                }
            }
            else if(i==26||i==28)
            {
                pChess->prop = STRONGHOLD;
            }
        }
    }
    for(i=0;i<9;i++)
    {
        pChess = &(pJunqi->aChessPos[4*nChess+i]);
        pChess->prop = RAILWAY;
    }
}

void InitBoardPoint(Junqi* pJunqi)
{
    int i,j;
    BoardChess *pChess;
    int nChess = 30;

    for(j=0;j<4;j++)
    {
        for(i=0;i<nChess;i++)
        {
            pChess = &pJunqi->aChessPos[j*nChess+i];
            switch(j)
            {
            case HOME:
                pChess->point.x = 10-i%5;
                pChess->point.y = 11+i/5;
                break;
            case RIGHT:
                pChess->point.x = 5-i/5;
                pChess->point.y = 10-i%5;
                break;
            case OPPS:
                pChess->point.x = 6+i%5;
                pChess->point.y = 5-i/5;
                break;
            case LEFT:
                pChess->point.x = 11+i/5;
                pChess->point.y = 6+i%5;
                break;
            default:
                assert(0);
                break;
            }
            pJunqi->apBoard[pChess->point.x][pChess->point.y] = pChess;
        }
    }
    for(i=0; i<9; i++)
    {
        pChess = &pJunqi->aChessPos[4*nChess+i];
        pChess->point.x = 10-(i%3)*2;
        pChess->point.y = 6+(i/3)*2;
        pJunqi->apBoard[pChess->point.x][pChess->point.y] = pChess;
    }
}

void InitChess(Junqi* pJunqi, u8 *data)
{
    InitLineup(pJunqi,data);

}
void InitBoard(Junqi* pJunqi)
{
    SetChessPosProperty(pJunqi);
    InitBoardPoint(pJunqi);
}

Junqi *JunqiOpen(void)
{
    Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
    memset(pJunqi, 0, sizeof(Junqi));
    pJunqi->pSkl = SkeletonOpen(pJunqi);
    InitBoard(pJunqi);

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

