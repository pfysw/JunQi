/*
 * proxy.c
 *
 *  Created on: Dec 23, 2021
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define REC_LEN 1000
#define SERVER_PORT 1234
#define CLINE_PORT 5678

struct sockaddr_in server_addr;
struct sockaddr_in clinet_addr;
int client_fd;

int NewUdpSocket(int port)
{
    int socket_fd;
    struct sockaddr_in local;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        printf("Create Socket Failed!\n");
        pthread_detach(pthread_self());
    }

    local.sin_family = AF_INET;
    local.sin_addr.s_addr=INADDR_ANY;
    local.sin_port = htons(port);

    if(bind(socket_fd, (struct sockaddr *)&local, sizeof(struct sockaddr) )<0)
    {
        printf("Bind Error!\n");
        pthread_detach(pthread_self());
    }
    return socket_fd;
}

void *RecvFromClinet(void *arg)
{
    int socket_fd;
    int rec_len;
    int from_len;
    char buf[REC_LEN]={0};
    struct sockaddr_in rec_addr;
    int firsr_rec = 0;

    from_len = sizeof(struct sockaddr_in);
    socket_fd = *((int*)arg);
    printf("arg %d\n",socket_fd);
    while(1){
        rec_len = recvfrom(socket_fd, buf, REC_LEN, 0, (struct sockaddr *)&rec_addr,&from_len);
        if(!firsr_rec){
            clinet_addr = rec_addr;
        }
        printf("rec ip %s port %d\n",inet_ntoa(rec_addr.sin_addr),htons(rec_addr.sin_port));
        printf("len %d\n",rec_len);
        sendto(client_fd, buf, rec_len, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    }
    close(socket_fd);
    return NULL;
}

void main()
{
    int socket_fd;
    int server_fd;
    int rec_len;
    char buf[REC_LEN]={0};
    struct sockaddr_in rec_addr;
    int from_len;
    pthread_t t1;

    from_len = sizeof(struct sockaddr_in);
    socket_fd = NewUdpSocket(CLINE_PORT);
    client_fd = socket_fd;
    printf("client_fd %d\n",socket_fd);
    while(1){
        rec_len = recvfrom(socket_fd, buf, REC_LEN, 0, (struct sockaddr *)&rec_addr,&from_len);
        printf("rec ip %s port %d\n",inet_ntoa(rec_addr.sin_addr),htons(rec_addr.sin_port));
        printf("len %d:%s\n",rec_len,buf);
        if(!strcmp(buf,"register！")){
            server_addr = rec_addr;
            printf("ok\n");
            sendto(socket_fd, "ok", 2, 0, (struct sockaddr *)&rec_addr, sizeof(struct sockaddr));
            break;
        }
    }
    server_fd = NewUdpSocket(SERVER_PORT);
    printf("server_fd %d\n",server_fd);
    pthread_create(&t1,NULL,(void*)RecvFromClinet,(void*)&server_fd);
    while(1){
        rec_len = recvfrom(socket_fd, buf, REC_LEN, 0, (struct sockaddr *)&rec_addr,&from_len);
        printf("rec ip %s port %d\n",inet_ntoa(rec_addr.sin_addr),htons(rec_addr.sin_port));
        printf("len %d\n",rec_len);
        sendto(server_fd, buf, rec_len, 0, (struct sockaddr *)&clinet_addr, sizeof(struct sockaddr));
    }
    close(socket_fd);
    pthread_join(t1,NULL);
}
