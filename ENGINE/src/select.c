#include "engine.h"
#include "junqi.h"
#include "movegen.h"
#include "path.h"
#include "search.h"
#include "evaluate.h"
#include <time.h>


void FindBestMove(
        Junqi *pJunqi,
        MoveSort *pHead,
        MoveSort **pResult,
        int type,
        int depth,
        u8 hasHead )
{
    MoveSort *p;
    MoveSort *pTemp;
    int aValue[5];
    int minValue;
    u8 index;
    u8 sortNum = 5;
    int i,j;

    if( pHead==NULL ) return;

    memset(pResult,0,sortNum*sizeof(MoveSort *));
    for(i=0; i<sortNum; i++)
    {
        aValue[i] = -INFINITY;
    }
    pthread_mutex_lock(&pJunqi->search_mutex);
    for(p=pHead; ; p=p->pNext)
    {

//        int index = p->pHead->index;
//        SafeMemout((u8*)&p->pHead->result[index].move, 4);
//        log_a("depthe %d type %d val %d",depth,type,p->aValue[depth][type]);

        if( !p->isSetValue[depth][type] )
        {
            goto continue_search;
        }

        minValue = INFINITY;
        for(i=0; i<sortNum; i++)
        {
            if( minValue>aValue[i] )
            {
                minValue = aValue[i];
                index = i;
            }
        }

        if( minValue<p->aValue[depth][type] )
        {
            aValue[index] = p->aValue[depth][type];
            pResult[index] = p;
        }
continue_search:
        if( hasHead )
        {
            if( p->pNext->isHead )
            {
                break;
            }
        }
        else
        {
            if( p->pNext==NULL )
            {
                break;
            }
        }
    }

    for(i=0; i<sortNum-1; i++)
    {
        for(j=i+1; j<sortNum; j++)
        {
            if( pResult[j]==NULL ) continue;

            if( pResult[i]==NULL)
            {
                pResult[i] = pResult[j];
            }
            else
            {
                if( pResult[i]->aValue[depth][type]<
                        pResult[j]->aValue[depth][type] )
                {
                    pTemp = pResult[j];
                    pResult[j] = pResult[i];
                    pResult[i] = pTemp;

                }
            }
        }
    }

    //assert(pResult[0]!=NULL);
    pthread_mutex_unlock(&pJunqi->search_mutex);


}

void SetSortRank(MoveSort *pHead)
{
    MoveSort *p;
    u8 rank = 0;

    if( pHead==NULL ) return;

    for(p=pHead; p!=NULL; p=p->pNext)
    {
        p->rank = rank;
        rank++;
    }
}

MoveSort * SelectRandMove(Junqi *pJunqi,MoveSort *pHead, int deepDepth)
{
    MoveSort *p;
    Engine *pEngine = pJunqi->pEngine;
    int maxLoop;
    int i = 0;


    if( pHead==NULL ) return NULL;
    maxLoop = pEngine->gInfo.timeStamp%5;
    log_a("timeStamp rand %d",maxLoop);
    for(p=pHead; p->pNext!=NULL; )
    {
        if( ++i>maxLoop )
        {
            break;
        }

        if( p->pNext->isSetValue[deepDepth][SEARCH_DEFAULT] )
        {
            if( p->aValue[deepDepth][SEARCH_DEFAULT]>
                p->pNext->aValue[deepDepth][SEARCH_DEFAULT]+50 )
            {
                break;
            }
        }
        p=p->pNext;
    }
    return p;
}

int CalMaxSumMinValue(
        Junqi *pJunqi,
        MoveSort *pNode,
        int deepDepth,
        int *pMinVal)
{
    int maxSum = 0;
    *pMinVal = 10000;
    for(int j=0; j<3; j++)
    {
        if( pNode->isSetValue[deepDepth][j] )
        {
            maxSum += pNode->aValue[deepDepth][j];
            if( *pMinVal>pNode->aValue[deepDepth][j] )
            {
                *pMinVal = pNode->aValue[deepDepth][j];
            }
        }
    }
//    if( pJunqi->nEat>20 && pJunqi->nNoEat<15 )
//    {
//        maxSum += pNode->aValue[0][DANGER_PATH];
//    }
//    if( pJunqi->nNoEat>15 )//todo 很危险
//    {
//        maxSum += pNode->aValue[2][SEARCH_SINGLE];
//    }

    return maxSum;
}

MoveSort *VerifyDeepMove(Junqi *pJunqi, MoveSort *pHead)
{
    MoveSort *pResult[5];
    MoveSort *pSelect[5];
    MoveSort *pNode;
    MoveSort *pBest;
    MoveSort *pInit = pHead;
    int index;
    int deepDepth = 3;
    u8 headCnt = 0;
    int  maxSum = 0;
    int minValue = 0;
    int min;
    int sum;
    u8 isDanger = 0;
    //MoveSort *pDebug[5];

    SetSortRank(pHead);
    if( pJunqi->nEat<5 )
    {
        pHead = SelectRandMove(pJunqi,pHead,deepDepth-1);

    }
    pBest = pHead;

    ReSearchInDeep(pJunqi,pHead,3);
//    sleep(1);
//    assert(0);

    for(int i=SEARCH_DEFAULT; i<3; i++)
    {
        FindBestMove(pJunqi,pInit,pResult,i,3,0);
        pNode = pResult[0];
//        if( i==0 && pJunqi->iRpOfst==243)
//        {
//            log_a("sdsd");
//            sleep(1);
//            assert(0);
//        }

        //pDebug[i] = pNode;
        pSelect[i] = pNode;
        if( pNode!=NULL && pNode!=pHead )
        {
            log_a("************");
            index = pNode->pHead->index;
            SafeMemout((u8*)&pNode->pHead->result[index].move, 4);
            log_a("i %d val %d",i,pNode->aValue[3][2]);
            ReSearchInDeep(pJunqi,pNode,3);
            log_a("i %d val %d",i,pNode->aValue[3][2]);
//            if( i==0 )
//            {
//                log_a("sdsd");
//                sleep(1);
//                assert(0);
//            }
        }
        else
        {
            if( pNode==NULL )
            {
                log_a("no pNode %d",i);
            }
            else
            {
                log_a("same %d",i);
            }
        }
    }

    log_a("new head %d",pJunqi->pEngine->gInfo.timeStamp);


    maxSum = CalMaxSumMinValue(pJunqi,pHead,deepDepth,&minValue);
    log_a("head sum %d min %d",maxSum,minValue);
//    log_a("head defalut %d",pHead->aValue[2][0]);
//    if( pHead->aValue[2][0]==-12 )
//    {
//        assert(0);
//    }
    for(int i=SEARCH_DEFAULT; i<3; i++)
    {
//        FindBestMove(pJunqi,pHead,pResult,i,3,0);
//        pNode = pResult[0];
        pNode = pSelect[i];
        if( pNode!=NULL )
        {
            index = pNode->pHead->index;
            SafeMemout((u8*)&pNode->pHead->result[index].move, 4);
            for(int j=0; j<3; j++)
            {
                log_a("type %d val %d rank %d",j,
                        pNode->aValue[deepDepth][j],pNode->rank);
            }
        }
        if( pHead->isSetValue[deepDepth][i] )
        {
            log_a("pHead type %d val %d",i,pHead->aValue[deepDepth][i]);
        }
        if( pNode!=NULL && pNode!=pHead )
        {
            sum = CalMaxSumMinValue(pJunqi,pNode,deepDepth,&min);
            log_a("type %d sum %d min %d",i,sum,min);
            if( min>minValue+50 && min>-800 )
            {
                log_a("min update");
                maxSum = sum;
                pBest = pNode;
                minValue = min;
                isDanger = 1;
            }
            else if( sum>maxSum )
            {
                log_a("sum update");
                maxSum = sum;
                pBest = pNode;
                minValue = min;
            }
        }
        else if( pHead==pNode)
        {
            log_a("is best %d",i);
            headCnt++;
        }


    }

   // if( (headCnt>1 || pDefalut==pHead) )//&& subValue<80 )
    if( headCnt>1 && !isDanger )
    {
        pBest = pHead;
    }


    return pBest;

}
