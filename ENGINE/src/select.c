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
      //  assert(p->pNext!=pHead);
//        int index;
//        index = p->pHead->index;
//        SafeMemout((u8*)&p->pHead->result[index].move, 4);
        //if(p->pNext->aValue[type]+10<p->aValue[type] )
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

MoveSort *VerifyDeepMove(Junqi *pJunqi, MoveSort *pHead)
{
    MoveSort *pResult[5];
    MoveSort *pNode;
    MoveSort *pBest = pHead;
    int index;
    int deepDepth = 3;
    u8 isUpdate = 0;
    int delta = 0;
    u8 headCnt = 0;
    int subValue = 0;
    MoveSort *pDefalut;
    int  maxSum = 0;
    int sum;

    SetSortRank(pHead);

    ReSearchInDeep(pJunqi,pHead,3);
    for(int i=SEARCH_DEFAULT; i<3; i++)
    {
        FindBestMove(pJunqi,pHead,pResult,i,3,0);
        pNode = pResult[0];
        if( pNode!=NULL && pNode!=pHead )
        {
//            log_a("************");
//            log_a("i %d val %d",i,pNode->aValue[3][1]);
            ReSearchInDeep(pJunqi,pNode,3);
          //  log_a("i %d val %d",i,pNode->aValue[3][1]);
        }
        else
        {
            if( pNode==NULL )
            {
                log_a("no pNode %d",i);
            }
            else
            {
                log_a("samr %d",i);
            }
        }
    }


    maxSum = 0;
    for(int j=0; j<3; j++)
    {
        maxSum += pHead->aValue[deepDepth][j];
    }
    log_a("head sum %d",maxSum);
    for(int i=SEARCH_DEFAULT; i<3; i++)
    {
        FindBestMove(pJunqi,pHead,pResult,i,3,0);
        pNode = pResult[0];
        if( i==SEARCH_DEFAULT )
        {
            pDefalut = pNode;
        }
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
            sum = 0;
            for(int j=0; j<3; j++)
            {
                sum += pNode->aValue[deepDepth][j];
            }
            log_a("type %d sum %d",i,sum);
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
