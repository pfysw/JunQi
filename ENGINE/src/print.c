/*
 * print.c
 *
 *  Created on: Sep 22, 2018
 *      Author: Administrator
 */
#include "utiliy.h"
#include "comm.h"
#include "engine.h"
#include "junqi.h"

#define PRINT_MSG  0
#define MEMOUT_MSG 1

typedef struct PrintMsg
{
	u8 type;
	u8 data[1];
}PrintMsg;

Junqi* gJunqi;

#if 1
void memout(u8 *pdata,int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		printf("%02X ",*(pdata+i));
		if((i+1)%8==0)
		{
			printf("\n");
		}
	}
	printf("\n");
}
#endif
////////////////////////////////

void *print_thread(void *arg)
{
	int len;
	u8 aBuf[REC_LEN];
	Junqi* pJunqi = (Junqi*)arg;
	PrintMsg *pData;

    while (1)
    {
    	len = mq_receive(pJunqi->print_qid, (char *)aBuf, REC_LEN, NULL);
        if ( len > 0)
        {
        	pData = (PrintMsg *)aBuf;
        	switch(pData->type)
        	{
        	case PRINT_MSG:
        		aBuf[len] = '\0';
        		printf("%s",pData->data);
        		break;
        	case MEMOUT_MSG:
        		memout(pData->data,len-1);
        		break;
        	default:
        		break;
        	}
        }
    }

	pthread_detach(pthread_self());
	return NULL;
}

void SafePrint(const char *zFormat, ...)
{

	va_list ap;
	char zBuf[50];
	int len;
	Junqi* pJunqi = gJunqi;
	PrintMsg *pData;

	va_start(ap,zFormat);
	len = vsprintf(zBuf, zFormat, ap);
	pData = (PrintMsg *)malloc(len+1);
	pData->type = PRINT_MSG;
	memcpy(pData->data,zBuf,len);
	mq_send(pJunqi->print_qid, (char*)pData, len+1, 0);
	free(pData);
	va_end(ap);
}

void SafeMemout(u8 *aBuf,int len)
{
	PrintMsg *pData;
	Junqi* pJunqi = gJunqi;

	pData = (PrintMsg *)malloc(len+1);
	pData->type = MEMOUT_MSG;
	memcpy(pData->data,aBuf,len);
	mq_send(pJunqi->print_qid, (char*)pData, len+1, 0);
	free(pData);
}

pthread_t CreatPrintThread(Junqi* pJunqi)
{
    pthread_t tidp;
    mqd_t qid;
    struct mq_attr attr = {0,5,REC_LEN};


    mq_unlink("print_msg");
    qid = mq_open("print_msg", O_CREAT | O_RDWR, 644, &attr);
    if (qid == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    pJunqi->print_qid = qid;

    pthread_create(&tidp,NULL,(void*)print_thread,pJunqi);
    return tidp;
}
