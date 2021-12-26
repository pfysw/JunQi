/*
 * comm.c
 *
 *  Created on: Oct 10, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include "skeleton.h"

char server_ip[30] = "172.0.0.1";

void PacketHeader(CommHeader *header, u8 iDir, u8 eFun)
{
    memset(header, 0, sizeof(CommHeader));
    memcpy(header->aMagic, aMagic, 4);
    //目前该位只在COMM_START标识先手位
    header->iDir = iDir;
    header->eFun = eFun;
}

void SendData(Junqi* pJunqi, CommHeader *header, void *data, int len)
{
    u8 buf[1000];
    int length = 0;

    length += sizeof(CommHeader);
    memcpy(buf, header, length);

    memcpy(buf+length, data, len);
    length += len;

    sendto(pJunqi->socket_fd, buf, length, 0,
            (struct sockaddr *)&pJunqi->addr, sizeof(struct sockaddr));
    if(gDebug.flagComm){
        log_a("send %d",header->eFun);
    }
}

void SendHeader(Junqi* pJunqi, u8 iDir, u8 eFun)
{
    CommHeader header;
    PacketHeader(&header, iDir, eFun);
    SendData(pJunqi, &header, NULL, 0);
}

void InitServerIp(int argc, char *argv[])
{
    if(argc>1){
        strcpy(server_ip,argv[1]);
    }
    printf("server ip %s\n",server_ip);
}

void DealRecData(Junqi* pJunqi, u8 *data, int len)
{
    CommHeader *pHead;
    pHead = (CommHeader *)data;

    if( memcmp(pHead->aMagic, aMagic, 4)!=0 )
    {
        return;
    }

    if(gDebug.flagComm){
        log_a("rec fun %d len %d",pHead->eFun,len);
    }

    switch(pHead->eFun)
    {
    case COMM_READY:
        break;
    case COMM_INIT:
        log_a("init");
        InitChess(pJunqi,data);
        CheckChessInit(pJunqi->pSkl);
        break;
    default:
        break;
    }
}

void InitUdpSocket(Junqi* pJunqi)
{

    int socket_fd;
    struct sockaddr_in addr,local;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        printf("Create Socket Failed!\n");
        pthread_detach(pthread_self());
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr=INADDR_ANY;
#ifdef  TEST
    local.sin_port = htons(6678);
#else
    local.sin_port = htons(5678);
#endif
    if(bind(socket_fd, (struct sockaddr *)&local, sizeof(struct sockaddr) )<0)
    {
        printf("Bind Error!\n");
        pthread_detach(pthread_self());
    }

    addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &addr.sin_addr);
    addr.sin_port = htons(1234);

    pJunqi->socket_fd = socket_fd;
    pJunqi->addr = addr;
    SendHeader(pJunqi, ENGINE_DIR, COMM_READY);
}

void *comm_thread(void *arg)
{
    Junqi* pJunqi = (Junqi*)arg;
    size_t recvbytes = 0;
    u8 buf[REC_LEN]={0};

    printf("comm thread\n");
    InitUdpSocket(pJunqi);
    while(1){
        recvbytes=recvfrom(pJunqi->socket_fd, buf, REC_LEN, 0,NULL ,NULL);
        DealRecData(pJunqi, buf, recvbytes);
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
