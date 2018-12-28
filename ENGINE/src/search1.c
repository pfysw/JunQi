/*
 * search1.c
 *
 *  Created on: Oct 12, 2018
 *      Author: Administrator
 */

#include "engine.h"
#include "junqi.h"
#include "movegen.h"
#include "path.h"
#include "search.h"
#include "evaluate.h"
#include <time.h>

//与AlphaBeta不同的是每一次搜索不用先生成全部着法
//每生成一步立刻进行递归，出现剪枝时后面的着法也就不用生成了
//这样节省了大量InsertMove的调用

typedef struct AlphaBetaData
{
    int depth;
    int alpha;
    int beta;
    u8 aInitBest[4];
    MoveResultData *pBest;
    MoveList *pCur;
    MoveList *pHead;
    int mxVal;
    int iDir;
    u8 cnt;
    u8 cut;
    u8 bestFlag;
}AlphaBetaData;


int CallAlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir,
        u8 isMove )
{
    int val;

    if( iDir%2==pJunqi->eTurn%2 )
    {
        //下家阵亡轮到对家走
        if( isMove )
            val = AlphaBeta1(pJunqi,depth,alpha,beta);
        else
            val = AlphaBeta1(pJunqi,depth,alpha,INFINITY);
    }
    else
    {
        if( isMove )
            val = -AlphaBeta1(pJunqi,depth,-beta,-alpha);
        else
            val = -AlphaBeta1(pJunqi,depth,-beta,INFINITY);
    }

    return val;
}

void UpdateBestToSort(
        Junqi *pJunqi,
        BestMove *aBestMove,
        BestMoveList *p,
        void *pSrc,
        u8 flag )
{
    BestMoveList *pRslt;
    MoveList *pMove;

    if( !flag )
    {
        pMove = (MoveList *)pSrc;
    }
    else
    {
        pRslt = (BestMoveList *)pSrc;
    }

    if(!flag)
    {
        SetBestMoveNode(aBestMove,p,pMove,1);
    }
    else
    {

        memcpy(p->result,pRslt->result,sizeof(p->result));
        p->index = pRslt->index;
//        u8 tet[4] = {0};
//        if( !memcmp(&p->result[p->index].move,tet,4) )
//        {
//            log_a("ds");
//        }
    }


    if( aBestMove[1].flag2 )
    {
        UpdateBestList(pJunqi, p, aBestMove[1].pHead,1);
    }
}

MoveSort *FindMoveSortList(
        MoveSort *pHead,
        void *pSrc,
        u8 flag
        )

{
    MoveSort *pNode = NULL;
    MoveResultData *pInput;
    MoveResultData *pData;
    BestMoveList *pRslt;
    MoveList *pMove;
    MoveSort *p;
    u8 index;

    if( !flag )
    {
        pMove = (MoveList *)pSrc;
        pInput = &pMove->move;
    }
    else
    {
        pRslt = (BestMoveList *)pSrc;
        pInput = &pRslt->result[pRslt->index].move;
    }

    assert(pHead!=NULL);
    for(p=pHead; ; p=p->pNext)
    {
        index = p->pHead->index;
        pData = &p->pHead->result[index].move;
        if( !memcmp(pData,pInput,4) )
        {
            pNode = p;
            break;
        }
        if( p->pNext->isHead )
        {
            break;
        }
    }

    return pNode;
}


void AddMoveSortList(
        Junqi *pJunqi,
        BestMove *aBestMove,
        void *pSrc,
        int value,
        u8 flag)
{

    Engine *pEngine = pJunqi->pEngine;
    MoveSort **ppHead = pEngine->ppMoveSort;
    MoveSort *pNode;
    //MoveSort *pMin;
    BestMoveList *p;
    int type = pJunqi->eSearchType;


    if( pJunqi->eSearchType==SEARCH_DEEP )
    {
        return;
    }

    pthread_mutex_lock(&pJunqi->search_mutex);

    if( *ppHead==NULL )
    {
        *ppHead = (MoveSort *)malloc(sizeof(MoveSort));
        //*ppHead = (MoveSort *)memsys5Malloc(pJunqi,sizeof(MoveSort));
        memset((*ppHead),0,sizeof(MoveSort));
        (*ppHead)->aValue[type] = value;
        (*ppHead)->isHead = 1;
        (*ppHead)->nNode = 1;
        (*ppHead)->pNext = *ppHead;
        (*ppHead)->pPre = *ppHead;
        *(pEngine->ppMoveSort) = *ppHead;

        p = (BestMoveList *)malloc(sizeof(BestMoveList));
        //p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));

        memset(p,0,sizeof(BestMoveList));
        (*ppHead)->pHead = p;

        UpdateBestToSort(pJunqi,aBestMove,p,pSrc,flag);

    }
    else
    {
        pNode = FindMoveSortList(*ppHead,pSrc,flag);
        if( pNode==NULL )
        {
            pNode = (MoveSort *)malloc(sizeof(MoveSort));
            //pNode = (MoveSort *)memsys5Malloc(pJunqi,sizeof(MoveSort));
            memset(pNode,0,sizeof(MoveSort));
            pNode->aValue[type] = value;
            pNode->pNext = *ppHead;
            pNode->pPre = (*ppHead)->pPre;
            (*ppHead)->pPre->pNext = pNode;
            (*ppHead)->pPre = pNode;
            (*ppHead)->nNode++;

            p = (BestMoveList *)malloc(sizeof(BestMoveList));
            //p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));

            memset(p,0,sizeof(BestMoveList));
            pNode->pHead = p;

            UpdateBestToSort(pJunqi,aBestMove,p,pSrc,flag);

        }
        else
        {
            pNode->aValue[type] = value;
            if( type==SEARCH_DEFAULT )
            {
                assert( pNode->pHead!=NULL  );

                FreeSortMoveNode(pJunqi,pNode->pHead);

                p = (BestMoveList *)malloc(sizeof(BestMoveList));
               // p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));
                memset(p,0,sizeof(BestMoveList));
                pNode->pHead = p;

                UpdateBestToSort(pJunqi,aBestMove,p,pSrc,flag);
            }
#if 0//查找最小分数节点
            pMin = pHead;
            for(pNode=pHead->pNext; !pNode->isHead; pNode=pNode->pNext)
            {
                if(pMin->value>pNode->value)
                {
                    pMin = pNode;
                }
            }
            if( value>pMin->value )
            {
                p = pMin->pHead;
                UpdateBestToSort(pJunqi,aBestMove,p,pSrc,flag);

                pMin->value = value;
            }
#endif
        }
    }

    pthread_mutex_unlock(&pJunqi->search_mutex);

}

//void ClearMoveSortList1(Junqi *pJunqi)
//{
//    Engine *pEngine = pJunqi->pEngine;
//    MoveSort *pHead = *(pEngine->ppMoveSort);
//    MoveSort *pNode;
//    MoveSort *pTemp;
//
//    pthread_mutex_lock(&pJunqi->search_mutex);
//
//    if(pHead==NULL)
//    {
//        return;
//    }
//
//    pNode=pHead->pNext;
//    while(1)
//    {
//        if( pNode->isHead )
//        {
//           // log_a("now free %d",pJunqi->free_cnt);
//            FreeSortMoveNode(pJunqi,pNode->pHead);
//            free(pNode);
//            break;
//        }
//        else
//        {
//         //   log_a("now free %d",pJunqi->free_cnt);
//            FreeSortMoveNode(pJunqi,pNode->pHead);
//            pTemp = pNode;
//            pNode = pNode->pNext;
//            free(pTemp);
//        }
//    }
//    *(pEngine->ppMoveSort) = NULL;
//
//    pthread_mutex_unlock(&pJunqi->search_mutex);
//}

void ClearMoveSortList(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    MoveSort *pTemp;

    if(pHead==NULL)
    {
        return;
    }

    pthread_mutex_lock(&pJunqi->search_mutex);

    log_a("head %d",pHead->isHead);
    if( pHead->isHead )
    {
        pHead->isHead = 0;
        pHead->pPre->pNext = NULL;//变为单向链表
    }
    pNode=pHead;
    while( pNode!=NULL )
    {
        FreeSortMoveNode(pJunqi,pNode->pHead);
        pTemp = pNode;
        pNode = pNode->pNext;
        free(pTemp);
    }
    *(pEngine->ppMoveSort) = NULL;

    pthread_mutex_unlock(&pJunqi->search_mutex);
}

int GetSearchTypeValue(Junqi *pJunqi, int type, int iDir)
{
    int val;
    switch(type)
    {
    case SEARCH_DEEP:
        val = ProSearch(pJunqi,4);
        break;
    case SEARCH_PATH:
        val = GetJunqiPathValue(pJunqi,iDir);
        break;
    default:
        break;
    }

    return val;
}

int DeepSearch(Junqi *pJunqi, BestMoveList *pNode, int type, int depth)
{
    int sum = 0;
    int val;

    int mxPerMove;
    int iDir = pJunqi->eTurn;

    if( pNode==NULL || depth==0 )
    {
        val = GetSearchTypeValue(pJunqi,type,iDir);
        return val;
    }

    mxPerMove = GetMaxPerMove(pNode->result);
    for(int i=0; i<RESULT_NUM; i++)
    {
        pJunqi->eTurn = iDir;
        if( !pNode->result[i].flag ) continue;

        if( i==mxPerMove )
        {
            MakeNextMove(pJunqi,&pNode->result[i].move);
            if( iDir%2==pJunqi->eTurn%2 )
            {
                val = DeepSearch(pJunqi,pNode->pNext,type,depth-1);
            }
            else
            {
                val = -DeepSearch(pJunqi,pNode->pNext,type,depth-1);
            }
            UnMakeMove(pJunqi,&pNode->result[i].move);

        }
        else
        {
            val = pNode->result[i].value;
        }

        if( i>0 )
        {
            sum += val*pNode->result[i].percent;
        }
        else
        {
            break;
        }
    }

    if( !pNode->result[MOVE-1].flag )
    {
        val = sum>>8;
    }
    pJunqi->eTurn = iDir;

    return val;
}

int SelectSortMove(Junqi *pJunqi)
{
    int mxVal = -INFINITY;
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    MoveSort *pBest;
    int val;

    log_a("\n****deep search*********");
    if(pHead==NULL)
    {
        return mxVal;
    }

    for(pNode=pHead; ;)
    {
        val = DeepSearch(pJunqi,pNode->pHead,SEARCH_DEEP,4);

        log_a("deep value %d",val);
        if( val>mxVal )
        {
            mxVal = val;
            pBest = pNode;
        }
        pNode=pNode->pNext;

       // break;
        if( pNode->isHead )
        {
            break;
        }
    }
    for(int i=0; i<RESULT_NUM; i++)
    {
        if( pBest->pHead->result[i].flag )
        {
            log_a("deep best %d",mxVal);
            SafeMemout((u8*)&pBest->pHead->result[i].move, sizeof(MoveResultData));
            SetBestMove(pJunqi,&pBest->pHead->result[i].move);
            break;
        }
    }

    return mxVal;
}


void SetPathValue(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    int val;

    if(pHead==NULL)
    {
        return;
    }

    for(pNode=pHead; ;)
    {
        val = DeepSearch(pJunqi,pNode->pHead,SEARCH_PATH,1);
        pNode->aValue[SEARCH_PATH] = val;

        pNode=pNode->pNext;

       // break;
        if( pNode->isHead )
        {
            break;
        }
    }
}


void PrintMoveSortList(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    u8 index;


    if(pHead==NULL)
    {
        return;
    }
    pHead->isHead = 0;
    pHead->pPre->pNext = NULL;//变为单向链表
    pHead = SortMoveValueList(pHead,SEARCH_PATH);
    *(pEngine->ppMoveSort) = pHead;

    log_a("move sort:****************");
    for(pNode=pHead; ;)
    {
        index = pNode->pHead->index;
       // PrintBestMove(pNode->pHead,pNode->aValue[type]);
        SafeMemout((u8*)&pNode->pHead->result[index].move, 4);
        log_a("default %d",pNode->aValue[SEARCH_DEFAULT]);
//        log_a("left %d",pNode->aValue[SEARCH_LEFT]);
//        log_a("right %d",pNode->aValue[SEARCH_RIGHT]);
        log_a("single %d",pNode->aValue[SEARCH_SINGLE]);
        log_a("path %d",pNode->aValue[SEARCH_PATH]);
        pNode=pNode->pNext;
        if( pNode==NULL )
        {
            break;
        }
    }
}

MoveSort *GetSortListEnd(MoveSort *pHead)
{
    MoveSort *p;

    if( pHead==NULL ) return NULL;
    for(p=pHead; p->pNext!=NULL; p=p->pNext);
    return p;
}

MoveSort *GetSortSameNodeEnd(MoveSort *pHead, int type)
{
    MoveSort *p;

    if( pHead==NULL ) return NULL;

    for(p=pHead; p->pNext!=NULL; p=p->pNext)
    {
        //if(p->pNext->aValue[type]+10<p->aValue[type] )
        if(p->pNext->aValue[type]!=p->aValue[type] )
        {
            break;
        }
    }
    return p;
}

void AdjustSortMoveValue(MoveSort *pHead, int type)
{
    MoveSort *p;

    if( pHead==NULL ) return;


    for(p=pHead; p!=NULL; p=p->pNext)
    {
        if(p->pHead->index==1)
        {
            p->aValue[type] += 50;
        }
    }
}

void CalSortSumValue(MoveSort *pHead, int type)
{
    MoveSort *p;
    int i;

    if( pHead==NULL ) return;


    for(p=pHead; p!=NULL; p=p->pNext)
    {
        for(i=0;i<type;i++)
        {
            if( i!=SEARCH_PATH )
            {
                p->aValue[type] += p->aValue[i];
            }
            else
            {
                p->aValue[type] += p->aValue[i]>>2;
            }
        }
    }
}

MoveSort *ResortMoveList(MoveSort *pHead, int depth)
{
    SearchType type;
    MoveSort *pTmep;
    MoveSort *pEnd;

    switch(depth)
    {
    case 0:
        type = SEARCH_DEFAULT;
        break;
    case 1:
        type = SEARCH_LEFT;
        break;
    case 2:
        type = SEARCH_RIGHT;
        break;
    case 3:
        type = SEARCH_SINGLE;
        break;
    case 4:
        type = SEARCH_PATH;
        break;
    default:
        return pHead;
    }
    pHead = SortMoveValueList(pHead,type);
    pEnd = GetSortSameNodeEnd(pHead,type);
    pTmep = pEnd->pNext;
    pEnd->pNext = NULL;
    pHead = ResortMoveList(pHead,depth+1);
    pEnd = GetSortListEnd(pHead);
    pEnd->pNext = pTmep;

    return pHead;
}

void FindBestPathMove(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    u8 index;


    if(pHead==NULL)
    {
        return;
    }
    pHead->isHead = 0;
    pHead->pPre->pNext = NULL;//变为单向链表

    if( pJunqi->nNoEat>10  )
    {
        AdjustSortMoveValue(pHead,SEARCH_DEFAULT);
    }

    CalSortSumValue(pHead,SEARCH_SUM);

    pHead = SortMoveValueList(pHead,SEARCH_SUM);

    //pHead = ResortMoveList(pHead,0);

//    pHead = SortMoveValueList(pHead,SEARCH_DEFAULT);
//    pEnd = GetSortSameNodeEnd(pHead,SEARCH_DEFAULT);
//    pTmep = pEnd->pNext;
//    pEnd->pNext = NULL;
//    pHead = SortMoveValueList(pHead,SEARCH_SINGLE);
//    pEnd = GetSortListEnd(pHead);
//    pEnd->pNext = pTmep;
//    pEnd = GetSortSameNodeEnd(pHead,SEARCH_SINGLE);
//    pTmep = pEnd->pNext;
//    pEnd->pNext = NULL;
//    pHead = SortMoveValueList(pHead,SEARCH_PATH);
//    pEnd = GetSortListEnd(pHead);
//    pEnd->pNext = pTmep;
    *(pEngine->ppMoveSort) = pHead;

    log_a("move sort:****************");
    for(pNode=pHead; ;)
    {
        index = pNode->pHead->index;
       // PrintBestMove(pNode->pHead,pNode->aValue[type]);
        SafeMemout((u8*)&pNode->pHead->result[index].move, 4);
        log_a("sum %d",pNode->aValue[SEARCH_SUM]);
        log_a("default %d",pNode->aValue[SEARCH_DEFAULT]);
        log_a("left %d",pNode->aValue[SEARCH_LEFT]);
        log_a("right %d",pNode->aValue[SEARCH_RIGHT]);
        log_a("single %d",pNode->aValue[SEARCH_SINGLE]);
        log_a("path %d",pNode->aValue[SEARCH_PATH]);
        pNode=pNode->pNext;
        if( pNode==NULL )
        {
            break;
        }
    }
    index = pHead->pHead->index;
    SetBestMove(pJunqi,&pHead->pHead->result[index].move);
}

void SearchAlphaBeta(
        Junqi *pJunqi,
        AlphaBetaData *pData
        )
{
    int depth = pData->depth;
    int alpha = pData->alpha;
    int beta = pData->beta;
    int cnt = pData->cnt;

    MoveResultData *pRslt;
    MoveList *p;
    int val;
    int sum = 0;
    BestMove *aBestMove = pJunqi->pEngine->aBestMove;
    int mxPercent;
#ifdef MOVE_HASH
    int iKey;
    int h;
#endif
    u8 isHashVal = 0;
    int pathValue;
    int saveValue;
    int orgValue;


    //注意SearchBestMove不能放前面
    //否则会破坏pJunqi->pMoveList这个全局临时变量
    if( pData->pCur==NULL )
    {
        pData->pCur = pJunqi->pMoveList;
        pData->pHead = pJunqi->pMoveList;
    }
    else
    {
        if( !pData->pCur->pNext->isHead )
        {
            pData->pCur = pData->pCur->pNext;
        }
        else
        {
            return;
        }
    }

    //如果这层是无棋可走，代码是不会运行到这里
    if( !pData->bestFlag )
    {
        pData->bestFlag = 1;
        pData->mxVal = SearchBestMove(pJunqi,aBestMove,cnt,alpha,beta,&pData->pBest,depth,1);
        if( pData->pBest!=NULL )
        {
            memcpy(pData->aInitBest,pData->pBest,4);
        }

        if( pData->mxVal>alpha )
        {
            pData->alpha = pData->mxVal;
            alpha = pData->mxVal;
        }
    }

    aBestMove[cnt].mxPerFlag1 = 1;
    mxPercent = 0;
    sum = 0;

    if( pData->pCur==NULL )
    {
        //在SearchBestMove被改变
        //由于进不去循环，所以在这里还原
        pJunqi->pMoveList = NULL;
    }

    for(p=pData->pCur; pData->pCur!=NULL; p=p->pNext)
    {

        pJunqi->eTurn = pData->iDir;
        //模拟着法产生后的局面
        //test(pJunqi,&p->move,pHead);

        MakeNextMove(pJunqi,&p->move);
        pJunqi->pMoveList = pData->pHead;
        assert(pJunqi->pEngine->pPos!=NULL);
#ifdef MOVE_HASH
        iKey = GetHashKey(pJunqi);
#endif
//        log_a("key %d",iKey);
//        SafeMemout((u8*)&p->move, sizeof(p->move));
        if ( pData->pBest!=NULL && !memcmp(pData->aInitBest, &p->move, 4) )
        {
            UnMakeMove(pJunqi,&p->move);
            goto continue_search;
        }
#ifdef MOVE_HASH
        else if( ( p->move.result==MOVE &&
              (h = CheckMoveHash(&pJunqi->paHash,iKey,depth,pJunqi->eTurn)) ) )
        {
            isHashVal = 1;
            val = pJunqi->paHash[h-1]->value;
            UnMakeMove(pJunqi,&p->move);
        }
#endif
        else
        {
            //val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,1);
            if( p->move.result==MOVE && cnt!=1 )
            {
                if( cnt==2 && pJunqi->aInfo[((pData->iDir-1)&3)].bDead )
                {
                    val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,0);
                }
                else
                {
                    val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,1);
                }


            }
            else
            {
                //碰撞中有3种情况，不能截断
                //第一层不截断，否则无法获取准确分数
                val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,0);
            }
            p->value = val;
#ifdef MOVE_HASH
            RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
#endif
//            u8 test1[4] = {0x08,0x0B,0x07,0x0C};
//            if(cnt<2 )
//            {
//                log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                SafeMemout((u8*)&p->move, sizeof(p->move));
//            }

            //把局面撤回到上一步
            pJunqi->pMoveList = pData->pHead;
            assert(pJunqi->pEngine->pPos!=NULL);
            UnMakeMove(pJunqi,&p->move);
        }

        if( p->move.result!=MOVE )
        {
            sum += val*p->percent;
            //log_a("sum %d val %d per %d",sum,val,p->percent);
            pRslt = &p->pNext->move;
            //下一个着法
            if( memcmp(&p->move, pRslt, 4) || p->pNext->isHead  )
            {
                aBestMove[cnt].mxPerFlag1 = 1;
                val = sum>>8;
                sum = 0;
            }
            else
            {
                //标记最大概率的碰撞结果
                //有3种结果，第一种始终置1
                //此后如果概率非最大值置0
                if( p->percent>mxPercent )
                {
                    mxPercent = p->percent;
                }
                if( p->pNext->percent>mxPercent )
                {
                    aBestMove[cnt].mxPerFlag1 = 1;
                }
                else
                {
                    aBestMove[cnt].mxPerFlag1 = 0;
                }
                goto continue_search;
            }

        }

        if( cnt==1 )
        {
            AddMoveSortList(pJunqi,aBestMove,p,val,0);
        }

        //产生截断
        if( val>=beta )//todo 被剪枝的最后可能分数不准确
        {
            if( -INFINITY==pData->mxVal && aBestMove[cnt-1].mxPerFlag1 )
            {
                UpdateBestMove(pJunqi,aBestMove,p,depth,cnt,isHashVal);

                pData->pBest = &p->move;
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val);
                }
            }
            pData->mxVal = val;
            pData->cut = 1;

            break;
        }

        if( val>pData->mxVal )
        {
//            if(depth==2&&cnt==3)
//            {
//                log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                SafeMemout((u8*)&p->move, sizeof(p->move));
//            }

#if 0
            //val>pData->mxVal要改成>=
            if( aBestMove[cnt-1].mxPerFlag1 )
            {
                if( aBestMove[cnt].flag2 )
                {
                    pathValue = aBestMove[cnt].pathValue;
                }
                else
                {
                    pathValue = GetJunqiPathValue(pJunqi,pData->iDir);
                }
                saveValue = aBestMove[cnt-1].pathValue;
                orgValue = pathValue;
                if( pData->iDir%2!=ENGINE_DIR%2 )
                {
                    pathValue = -pathValue;
                    saveValue = -saveValue;
                }
//                if(cnt==1)
//                log_a("dir %d pathValue:%d %d",pData->iDir,pathValue,saveValue);

                if(val+pathValue<pData->mxVal+saveValue)
                {
                    goto continue_search;
                }
                else
                {
                    aBestMove[cnt-1].pathValue = orgValue;
                }
            }
#endif

            pData->mxVal = val;
            if( aBestMove[cnt-1].mxPerFlag1 )
            {
                UpdateBestMove(pJunqi,aBestMove,p,depth,cnt,isHashVal);
                pData->pBest = &p->move;
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val);
                }
            }
            //更新alpha值
            if( val>alpha )
            {
                pData->alpha = val;
            }

        }

continue_search:
        isHashVal = 0;

        if( p->pNext->isHead )
        {
            break;
        }

        //时间结束或收到go指令结束搜索
        if( TimeOut(pJunqi) )
        {
            pData->cut = 1;
        }


    }


    pData->pCur = p;

}


void SearchRailPath1(
        Junqi* pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pDst,
        int flag,
        AlphaBetaData *pData )
{
    AdjNode *p;
    BoardGraph *pVertex;
    BoardChess *pChess = pSrc->pAdjList->pChess;


    pDst->cnt[pData->depth]++;

    for(p=pDst->pAdjList->pNext; p!=NULL; p=p->pNext)
    {
        if( pData->cut )
        {
            break;
        }
        pVertex = &pJunqi->aBoard[p->pChess->point.x][p->pChess->point.y];


        if( pVertex->cnt[pData->depth]!=0 )
        {
            continue;
        }
        //IsDirectRail必须要执行，所以放在前面
        else if( !IsDirectRail(pJunqi, pSrc, pVertex) && pChess->type!=GONGB )
        {
            continue;
        }
        else if( p->pChess->type!=NONE )
        {
            if( (p->pChess->pLineup->iDir&1)!=(pChess->pLineup->iDir&1) )
            {
                if( !flag )
                {

                    AddMoveToList(pJunqi, pChess, p->pChess);
                    pVertex->cnt[pData->depth]++;
                    //log_a("dir %d %d",p->pChess->iDir,pChess->iDir);
    //                log_a("dst %d %d %d %d",pChess->point.x,pChess->point.y,
    //                        p->pChess->point.x,p->pChess->point.y);

                    SearchAlphaBeta(pJunqi,pData);
                }
                else
                {
                    AddMoveToHash(pJunqi, pChess, p->pChess);
                }
            }
            continue;
        }
        else
        {
            if( !flag )
            {

                AddMoveToList(pJunqi, pChess, p->pChess);
                //log_a("path %d %d %d %d",pChess->point.x,pChess->point.y,
                       // p->pChess->point.x,p->pChess->point.y);
                SearchAlphaBeta(pJunqi,pData);

            }
            SearchRailPath1(pJunqi, pSrc, pVertex,flag,pData);
        }
    }
}


void ClearDepthCnt(Junqi *pJunqi,int depth)
{
    int i,j;

    for(i=0; i<17; i++)
    {
        for(j=0; j<17; j++)
        {
            pJunqi->aBoard[i][j].cnt[depth] = 0;
        }
    }
}

void SearchMoveList(
        Junqi* pJunqi,
        BoardChess *pSrc,
        int flag,
        AlphaBetaData *pData)
{

    BoardGraph *pVertex;
    BoardChess *pNbr;
    BoardGraph *pNbrVertex;

    int i,x,y;
    //log_c("clear");
    ClearDepthCnt(pJunqi,pData->depth);

    if( pSrc->isStronghold )
    {
        return;
    }
    else if( pSrc->type==NONE )
    {
        return;
    }
    else if( pSrc->type==DILEI || pSrc->type==JUNQI )
    {
        return;
    }

    if( pSrc->isRailway )
    {
        pVertex = &pJunqi->aBoard[pSrc->point.x][pSrc->point.y];
        SearchRailPath1(pJunqi, pVertex, pVertex, flag, pData);

    }
    for(i=0; i<9; i++)
    {
        if( pData->cut ) break;

        if( i==4 ) continue;
        x = pSrc->point.x+1-i%3;
        y = pSrc->point.y+i/3-1;

        if( x<0||x>16||y<0||y>16 ) continue;

        if( pJunqi->aBoard[x][y].pAdjList )
        {

            pNbr = pJunqi->aBoard[x][y].pAdjList->pChess;
            pNbrVertex = &pJunqi->aBoard[pNbr->point.x][pNbr->point.y];
            if( pNbrVertex->cnt[pData->depth]!=0 )
            {
                continue;
            }
            else if( pNbr->isCamp && pNbr->type!=NONE )
            {
                continue;
            }
            else if( pNbr->type!=NONE && (pNbr->pLineup->iDir&1)==(pSrc->pLineup->iDir&1) )
            {
                continue;
            }
            pNbr->isSapperPath = 0;
            if( pSrc->isCamp || pNbr->isCamp )
            {
                if( !flag )
                {

                    AddMoveToList(pJunqi, pSrc, pNbr);
    //                log_a("nbr1 %d %d %d %d",pSrc->point.x,pSrc->point.y,
    //                        pNbr->point.x,pNbr->point.y);

                    SearchAlphaBeta(pJunqi,pData);
                }
                else if( pNbr->type!=NONE )
                {
                    AddMoveToHash(pJunqi, pSrc, pNbr);
                }
            }
            //非斜相邻
            else if( pNbr->point.x==pSrc->point.x || pNbr->point.y==pSrc->point.y)
            {
                if( !flag )
                {
                    AddMoveToList(pJunqi, pSrc, pNbr);
    //                log_a("nbr2 %d %d %d %d",pSrc->point.x,pSrc->point.y,
    //                        pNbr->point.x,pNbr->point.y);
                    SearchAlphaBeta(pJunqi,pData);
                }
                else if( pNbr->type!=NONE )
                {
                    AddMoveToHash(pJunqi, pSrc, pNbr);
                }
            }

        }
    }

}

u8 SelectSearchType(Junqi *pJunqi, int myTurn, int iDir)
{
    u8 rc = 0;
    switch(pJunqi->eSearchType )
    {
    case SEARCH_RIGHT:
        if( iDir!=myTurn && iDir!=((myTurn+1)&3) )
        {
            rc = 1;
        }
        break;
    case SEARCH_LEFT:
        if( iDir!=myTurn && iDir!=((myTurn+3)&3) )
        {
            rc = 1;
        }
        break;
    case SEARCH_SINGLE:
        if( iDir!=myTurn )
        {
            rc = 1;
        }
        break;
    default:
        break;
    }

    return rc;
}

int AlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta)
{
    int i;
    BoardChess *pSrc;
    ChessLineup *pLineup;

    int val;

    static int myTurn;
    int iDir = pJunqi->eTurn;
    AlphaBetaData search_data;
    BestMove *aBestMove = pJunqi->pEngine->aBestMove;

    if( !pJunqi->cnt )
    {
        pJunqi->paHash = NULL;
        aBestMove[0].mxPerFlag = 1;
        aBestMove[0].mxPerFlag1 = 1;
        aBestMove[0].pNode = aBestMove[0].pHead;
        myTurn = iDir;
    }
    pJunqi->cnt++;

    search_data.depth = depth;
    search_data.alpha = alpha;
    search_data.beta = beta;
    search_data.cut = 0;
    search_data.pBest = NULL;
    search_data.iDir = iDir;
    search_data.pCur = NULL;
    search_data.pHead = NULL;
    search_data.mxVal = -INFINITY;
    search_data.cnt = pJunqi->cnt;
    search_data.bestFlag = 0;


    //遍历到最后一层时计算局面分值
    if( depth==0 )
    {
        val = EvalSituation(pJunqi);
        pJunqi->test_num++;
        //val = 5;
        //EvalSituation是针对引擎评价的，所以对方的分值应取负值
        if( iDir%2!=ENGINE_DIR%2 )
        {
            val = -val;
        }
        pJunqi->cnt--;
        //log_a("test num %d", pJunqi->test_num);
        return val;
    }

    //2方单独测试
    if( SelectSearchType(pJunqi,myTurn,iDir) )
    {
        ChessTurn(pJunqi);
        pJunqi->cnt--;
        val = CallAlphaBeta1(pJunqi,depth,alpha,beta,iDir,1);

        if( 0==pJunqi->cnt )
        {
            pJunqi->cnt = 0;
            SetBestMove(pJunqi,search_data.pBest);
        }
        ClearMoveList(pJunqi,search_data.pHead);

        return val;
    }


    pJunqi->pMoveList = NULL;
    for(i=0; i<30; i++)
    {

        if( search_data.cut ) break;
        pLineup = &pJunqi->Lineup[iDir][i];
        if( pLineup->bDead || pLineup->type==NONE )
        {
            continue;
        }
        pSrc = pLineup->pChess;
        SearchMoveList(pJunqi,pSrc,0,&search_data);
        val = search_data.mxVal;
    }

    if( NULL==pJunqi->pMoveList )
    {
        pJunqi->eTurn = iDir;
        ChessTurn(pJunqi);
        val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,iDir,1);

        //不要再搜下一层了
        //例如3层搜索时，刚开始是最好着法是3步棋
        //后来搜到2步棋的最好着法，第3步无棋可走
        //由于代码原因，在链表里的第3步仍然还在
        //第4层搜索时不能把这一步搜进去
        //*********************************
        //在SearchBestMove里，一层可能有多种碰撞结果
        //不是概率最大的一种，不要修改pNode
        //因为best的搜索只在概率最大的分支下面
        if( aBestMove[pJunqi->cnt-1].mxPerFlag )
        {
            aBestMove[pJunqi->cnt-1].flag2 = 0;
            aBestMove[0].pNode = NULL;
        }
    }

    pJunqi->cnt--;
    if( 0==pJunqi->cnt )
    {
        pJunqi->cnt = 0;
        //log_a("get best");
        SetBestMove(pJunqi,search_data.pBest);
#ifdef MOVE_HASH
        ClearMoveHash(&pJunqi->paHash);
#endif
    }
    ClearMoveList(pJunqi,search_data.pHead);

    return val;
}
