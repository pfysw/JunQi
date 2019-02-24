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
            if( *pMinVal>pNode->isSetValue[deepDepth][j] )
            {
                *pMinVal = pNode->aValue[deepDepth][j];
            }
        }
    }
    if( pJunqi->nEat>20 )
    {
        maxSum += pNode->aValue[0][SEARCH_PATH];
    }

    return maxSum;
}
MoveSort *VerifyDeepMove(Junqi *pJunqi, MoveSort *pHead)
{
    MoveSort *pResult[5];
    MoveSort *pSelect[5];
    MoveSort *pNode;
    MoveSort *pBest = pHead;
    int index;
    int deepDepth = 3;
    u8 headCnt = 0;
    int  maxSum = 0;
    int minValue = 0;
    int min;
    int sum;
    //MoveSort *pDebug[5];

    SetSortRank(pHead);

    ReSearchInDeep(pJunqi,pHead,3);
    for(int i=SEARCH_DEFAULT; i<3; i++)
    {
        FindBestMove(pJunqi,pHead,pResult,i,3,0);
        pNode = pResult[0];
//        if( i==1 )
//        {
//            pJunqi->bDebug = 1;
//            log_a("sdsd");
//        }
//        sleep(1);
//        assert(0);
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
//            if( i==1 )
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


    maxSum = CalMaxSumMinValue(pJunqi,pHead,deepDepth,&minValue);
    log_a("head sum %d min %d",maxSum,minValue);
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
            if( min>minValue+50 )
            {
                maxSum = sum;
                pBest = pNode;
            }
            if( sum>maxSum )
            {
                maxSum = sum;
                pBest = pNode;
            }
        }
        else if( pHead==pNode)
        {
            log_a("is best %d",i);
            headCnt++;
        }


    }

   // if( (headCnt>1 || pDefalut==pHead) )//&& subValue<80 )
    if( headCnt>1 )
    {
        pBest = pHead;
    }

    return pBest;

}
