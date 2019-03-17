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



#define CALL_ALPHA_TEST 1
int CallAlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir,
        u8 isMove )
{
    int val;



    if( iDir==pJunqi->eTurn && pJunqi->gFlag[FLAG_PREVENT]<2 )
    {
        depth = 0;
    }
    if( iDir%2==pJunqi->eTurn%2 )
    {
        //下家阵亡轮到对家走
        if( isMove )
            val = AlphaBeta1(pJunqi,depth,alpha,beta,isMove);
        else
#if CALL_ALPHA_TEST
            val = AlphaBeta1(pJunqi,depth,-INFINITY,INFINITY,isMove);
#else
            val = AlphaBeta1(pJunqi,depth,alpha,INFINITY,isMove);
#endif
    }
    else
    {
        if( isMove )
            val = -AlphaBeta1(pJunqi,depth,-beta,-alpha,isMove);
        else
#if CALL_ALPHA_TEST
            //为了防止吃子时下下层被剪枝
            val = -AlphaBeta1(pJunqi,depth,-INFINITY,INFINITY,isMove);
#else
            val = -AlphaBeta1(pJunqi,depth,-beta,INFINITY,isMove);
#endif
    }


//    if( pJunqi->bDebug==1 )
//    {
//        log_a("val %d dir %d cnt %d",val,iDir,pJunqi->cnt);
//    }
//    static int jj=0;
//    jj++;
//    if(jj==13)
//    log_a("test");
//    log_a("kk %d",jj);
    return val;
}

void UpdateBestToSort(
        Junqi *pJunqi,
        BestMove *aBestMove,
        BestMoveList *p,
        void *pSrc,
        MoveList *pMaxMove,
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
        SetBestMoveNode(p,pMove,pMaxMove);
    }
    else
    {
        memcpy(p->result,pRslt->result,sizeof(p->result));
        p->index = pRslt->index;
        p->isMove = pRslt->isMove;
//        u8 tet[4] = {0};
//        if( !memcmp(&p->result[p->index].move,tet,4) )
//        {
//            log_a("ds");
//        }
    }


//    if( aBestMove[1].pHead!=NULL &&
//            aBestMove[1].pHead->isMove )
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
        MoveList *pMaxMove,
        int value,
        u8 flag)
{

    Engine *pEngine = pJunqi->pEngine;
    MoveSort **ppHead = pEngine->ppMoveSort;
    MoveSort *pNode;
    //MoveSort *pMin;
    BestMoveList *p;
    int type = pJunqi->eSearchType;
    int depth = pJunqi->nDepth-1;//减1为了把0地址也用上


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
        (*ppHead)->aValue[depth][type] = value;
        (*ppHead)->isSetValue[depth][type] = 1;
        (*ppHead)->isHead = 1;
        (*ppHead)->nNode = 1;
        (*ppHead)->pNext = *ppHead;
        (*ppHead)->pPre = *ppHead;
        *(pEngine->ppMoveSort) = *ppHead;

        p = (BestMoveList *)malloc(sizeof(BestMoveList));
        //p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));

        memset(p,0,sizeof(BestMoveList));
        (*ppHead)->pHead = p;

        UpdateBestToSort(pJunqi,aBestMove,p,pSrc,pMaxMove,flag);

    }
    else
    {
        pNode = FindMoveSortList(*ppHead,pSrc,flag);
        if( pNode==NULL )
        {
            pNode = (MoveSort *)malloc(sizeof(MoveSort));
            //pNode = (MoveSort *)memsys5Malloc(pJunqi,sizeof(MoveSort));
            memset(pNode,0,sizeof(MoveSort));
            pNode->aValue[depth][type] = value;
            pNode->isSetValue[depth][type] = 1;
            pNode->pNext = *ppHead;
            pNode->pPre = (*ppHead)->pPre;
            (*ppHead)->pPre->pNext = pNode;
            (*ppHead)->pPre = pNode;
            (*ppHead)->nNode++;

            p = (BestMoveList *)malloc(sizeof(BestMoveList));
            //p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));

            memset(p,0,sizeof(BestMoveList));
            pNode->pHead = p;

            UpdateBestToSort(pJunqi,aBestMove,p,pSrc,pMaxMove,flag);

        }
        else
        {
            pNode->aValue[depth][type] = value;
            pNode->isSetValue[depth][type] = 1;
            if( type==SEARCH_DEFAULT )
            {
                assert( pNode->pHead!=NULL  );

                FreeSortMoveNode(pJunqi,pNode->pHead);

                p = (BestMoveList *)malloc(sizeof(BestMoveList));
               // p = (BestMoveList *)memsys5Malloc(pJunqi,sizeof(BestMoveList));
                memset(p,0,sizeof(BestMoveList));
                pNode->pHead = p;

                UpdateBestToSort(pJunqi,aBestMove,p,pSrc,pMaxMove,flag);
            }

        }
    }

    pthread_mutex_unlock(&pJunqi->search_mutex);

}

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
    int depth;

    depth = pJunqi->pEngine->gInfo.mxDepth-1;

    switch(type)
    {
    case SEARCH_DEEP:
        if( pJunqi->eDeepType!=SEARCH_SINGLE )
        {
            val = ProSearch(pJunqi,depth);
        }
        else
        {
            //对于single来由于没有剪枝，搜索4层太长了
            val = ProSearch(pJunqi,3);
        }
        break;
    case SEARCH_PATH:
        val = GetJunqiPathValue(pJunqi,iDir,0);
        break;
    case DANGER_PATH:
        val = GetJunqiPathValue(pJunqi,iDir,1);
        break;
    case SEARCH_CONNECT:
        val = GetConnectValue(pJunqi,iDir);
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

        if( i==mxPerMove || depth==1 )
        {
            MakeNextMove(pJunqi,&pNode->result[i].move,NULL);
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
        if( pJunqi->nEat>10 )//开局不要搜
        {
            pJunqi->myTurn = pJunqi->eTurn;
            val = DeepSearch(pJunqi,pNode->pHead,SEARCH_PATH,1);
            pNode->aValue[0][SEARCH_PATH] = val;

            pJunqi->myTurn = pJunqi->eTurn;
            val = DeepSearch(pJunqi,pNode->pHead,DANGER_PATH,1);
            pNode->aValue[0][DANGER_PATH] = val;
        }



        pJunqi->myTurn = pJunqi->eTurn;
        val = DeepSearch(pJunqi,pNode->pHead,SEARCH_CONNECT,1);
        pNode->aValue[0][SEARCH_CONNECT] = val;

        pNode=pNode->pNext;

       // break;
        if( pNode->isHead )
        {
            break;
        }
    }
}

#if 0
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
#endif

MoveSort *GetSortListEnd(MoveSort *pHead)
{
    MoveSort *p;

    if( pHead==NULL ) return NULL;
    for(p=pHead; p->pNext!=NULL; p=p->pNext);
    return p;
}

MoveSort *GetSortSameNodeEnd(MoveSort *pHead, int type, int depth)
{
    MoveSort *p;

    if( pHead==NULL ) return NULL;

    for(p=pHead; p->pNext!=NULL; p=p->pNext)
    {
//        log_a("***type %d depth %d val %d",type,depth,p->aValue[depth][type]);
//        int index = p->pHead->index;
//        SafeMemout((u8*)&p->pHead->result[index].move, 4);

        if(p->pNext->aValue[depth][type]!=p->aValue[depth][type] )
        {
            break;
        }
    }
    return p;
}

void AdjustSortMoveValue(MoveSort *pHead, int type, int depth)
{
    MoveSort *p;

    if( pHead==NULL ) return;


    for(p=pHead; p!=NULL; p=p->pNext)
    {
        if(p->pHead->index==1)
        {
            p->aValue[depth][type] += 50;
        }
    }
}

void CalSortSumValue(
        Junqi *pJunqi,
        MoveSort *pHead,
        int type,
        int depth)
{
    MoveSort *p;
    int i,j;

    if( pHead==NULL ) return;


    for(j=0; j<=depth; j++)
    {
        for(p=pHead; p!=NULL; p=p->pNext)
        {
            for(i=0;i<SEARCH_SINGLE;i++)
            {
                p->aValue[j][type] += p->aValue[j][i];
            }
//            if( pJunqi->iRpOfst<100 || pJunqi->nNoEat<10 )
//            {
//                p->aValue[j][type] += p->aValue[0][DANGER_PATH];
//            }
//            if( pJunqi->nNoEat>15 )//todo 很危险
//            {
//                p->aValue[j][type] += p->aValue[j][SEARCH_SINGLE];
//            }
        }
    }
}

MoveSort *ResortMoveList(MoveSort *pHead, int pri, int depth)
{
    SearchType type;
    MoveSort *pTmep;
    MoveSort *pEnd;
    static u8 cnt = 0;
    static u8 firstPri;
    static u8 firstType;

    cnt++;

    switch(pri)
    {
    case 0:
        type = SEARCH_SUM;
        break;
    case 1:
        type = SEARCH_DEFAULT;
        break;
    case 2:
        type = SEARCH_RIGHT;
        break;
    case 3:
        type = SEARCH_LEFT;
        break;
    case 4:
        type = SEARCH_SINGLE;
        break;
    case 5:
        type = DANGER_PATH;
        break;
    case 6:
        type = SEARCH_PATH;
        break;
    case 7:
        type = SEARCH_CONNECT;
        break;
    default:
        if( depth==0 )
        {
            cnt--;
            return pHead;
        }
        else
        {
            type = firstType;
            pri = firstPri;
            depth--;
        }

        break;
    }

    if( cnt==1 )
    {
        assert( pri<8 );
        firstPri = pri;
        firstType = type;
    }


    if( type!=SEARCH_PATH &&
            type!=DANGER_PATH &&
            type!=SEARCH_CONNECT )
    {
        pHead = SortMoveValueList(pHead,type,depth);
        pEnd = GetSortSameNodeEnd(pHead,type,depth);
    }
    else
    {
        pHead = SortMoveValueList(pHead,type,0);
        pEnd = GetSortSameNodeEnd(pHead,type,0);
    }

    pTmep = pEnd->pNext;
    pEnd->pNext = NULL;
    if( SEARCH_SINGLE==type && depth!=0 )
    {
        pri += 100;//跳到default
    }
    pHead = ResortMoveList(pHead,pri+1,depth);
    pEnd = GetSortListEnd(pHead);
    pEnd->pNext = pTmep;

    cnt--;
    return pHead;
}

void FindBestPathMove(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    MoveSort *pHead = *(pEngine->ppMoveSort);
    MoveSort *pNode;
    MoveSort *pBest;
    u8 index;
    int depth;

    depth = 3-1;//第3层，0地址为第1层
    if(pHead==NULL)
    {
        return;
    }
    pHead->isHead = 0;
    pHead->pPre->pNext = NULL;//变为单向链表

    if( pJunqi->nNoEat>10  )
    {
        AdjustSortMoveValue(pHead,SEARCH_DEFAULT,depth);
    }

    CalSortSumValue(pJunqi,pHead,SEARCH_SUM,depth);

    //pHead = SortMoveValueList(pHead,SEARCH_SUM);
    //pHead = SortMoveValueList(pHead,SEARCH_RIGHT);
    //pHead = SortMoveValueList(pHead,SEARCH_SINGLE,depth);

    pHead = ResortMoveList(pHead,0,depth);
    //pJunqi->eDeepType = SEARCH_RIGHT;
    pBest = VerifyDeepMove(pJunqi,pHead);

    *(pEngine->ppMoveSort) = pHead;

    log_a("move sort:****************");

    for(pNode=pHead; ;)
    {
        index = pNode->pHead->index;
       // PrintBestMove(pNode->pHead,pNode->aValue[type]);
        SafeMemout((u8*)&pNode->pHead->result[index].move, 4);

        for(int i=depth; i>=depth; i--)
        {
            log_a("sum[%d] %d",i,pNode->aValue[depth][SEARCH_SUM]);
            log_a("default[%d] %d",i,pNode->aValue[i][SEARCH_DEFAULT]);
            log_a("left[%d] %d",i,pNode->aValue[i][SEARCH_LEFT]);
            log_a("right[%d] %d",i,pNode->aValue[i][SEARCH_RIGHT]);
            log_a("single[%d] %d",i,pNode->aValue[i][SEARCH_SINGLE]);
        }
        log_a("danger %d",pNode->aValue[0][DANGER_PATH]);
        log_a("path %d",pNode->aValue[0][SEARCH_PATH]);
        log_a("connect %d",pNode->aValue[0][SEARCH_CONNECT]);
        pNode=pNode->pNext;
        if( pNode==NULL )
        {
            break;
        }
    }
    index = pBest->pHead->index;
    SetBestMove(pJunqi,&pBest->pHead->result[index].move);

    ReAdjustMaxType(pJunqi);
}

u8 IsSearchAllMove(Junqi *pJunqi, int cnt, int iDir)
{
    u8 rc = 0;
    if( pJunqi->nDepth<4 && pJunqi->eSearchType!=SEARCH_DEEP )
    {
        if( cnt==1 || ( cnt==2 &&
                pJunqi->aInfo[((iDir-1)&3)].bDead ) )
        {
            rc = 1;
        }
    }

    return rc;
}

//todo 这段代码为了解决送子问题
//但是所消耗的搜索时间太多
//所以如果不影响胜负，暂时不用
void CheckPreventMove(
        Junqi *pJunqi,
        MoveList *p,
        u8 *flag)
{

    Engine *pEngine = pJunqi->pEngine;

    if( 2==pJunqi->gFlag[FLAG_PREVENT] )
    {
        return;
    }
    if( pJunqi->cnt==1 && pJunqi->eSearchType!=SEARCH_DEEP )
    {
        pEngine->pFirstMove = p;
    }
    else
    {
        //本想在最后选择时验证送吃是否合理
        //但现在这个标志位已经用来残局的关键步加深搜索
        return;

        if( pJunqi->eSearchType!=SEARCH_DEEP )
        {
            return;//todo 暂时不要
        }
        if( p->move.result==EAT && p->percent==256 &&
                pJunqi->eSearchType!=SEARCH_DEFAULT )
        {
            //故意送吃阻挡时，多搜几层
            if( !memcmp(pEngine->pFirstMove->move.dst,p->move.dst,2) )
            {
                pJunqi->gFlag[FLAG_PREVENT] = 1;
                *flag = 1;
            }
        }
    }
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
    MoveList *pMaxMove;
    MoveList *pNextMove;
    Engine *pEngine = pJunqi->pEngine;
    int val;
    int sum = 0;
    BestMove *aBestMove = pEngine->aBestMove;
    int mxPercent;
    u8 preventFlag = 0;

    //注意SearchBestMove不能放前面
    //否则会破坏pJunqi->pMoveList这个全局临时变量
    if( pData->pCur==NULL )
    {
        if( pJunqi->pMoveList==NULL )
        {
            return;
        }
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
    if( !pData->bestFlag && !pData->isGongB )
    {
        pData->bestFlag = 1;

        pData->mxVal = SearchBestMove(pJunqi,aBestMove,pData,&pData->bestMove,1);

        if( pData->hasBest )
        {
            memcpy(pData->aInitBest,&pData->bestMove.move,4);
        }

        if( pData->mxVal>alpha )
        {
            pData->alpha = pData->mxVal;
            alpha = pData->mxVal;
        }
    }

    aBestMove[cnt].mxPerFlag1 = 1;
    pNextMove = pData->pCur;

    mxPercent = 0;
    sum = 0;

    if( pData->pCur==NULL )
    {
        //在SearchBestMove被改变
        //由于进不去循环，所以在这里还原
        pJunqi->pMoveList = NULL;
    }

    //下一层不一定有最佳着法
    //左右搜索时会把一些着法过滤掉导致无棋可走
    //出现无棋可走时，这里防止把以前搜过着法加进来
    if( aBestMove[cnt].pHead!=NULL )
    {
        aBestMove[cnt].pHead->isMove = 0;
    }
    for(p=pData->pCur; pData->pCur!=NULL; p=p->pNext)
    {

        pMaxMove = pNextMove;
        pJunqi->eTurn = pData->iDir;

        CheckPreventMove(pJunqi,p,&preventFlag);

        //标记作用，无棋可走时避免无限递归，在single搜索时会发生
        pJunqi->bDead = 0;
        //模拟着法产生后的局面

        MakeNextMove(pJunqi,&p->move,&preventFlag);
        if( pJunqi->bDead )
        {
            depth = 1;
        }

        pJunqi->pMoveList = pData->pHead;
        assert(pJunqi->pEngine->pPos!=NULL);

//        log_a("key %d",iKey);
//        SafeMemout((u8*)&p->move, sizeof(p->move));
        if( pData->hasBest && !memcmp(pData->aInitBest, &p->move, 4) )
        {
            if( preventFlag )
            {
                pJunqi->gFlag[FLAG_PREVENT] = 0;
            }
            UnMakeMove(pJunqi,&p->move);
            goto continue_search;
        }
        else
        {
            assert(cnt==pJunqi->cnt);

            if( p->move.result==EAT )
            {
                pData->hasEat = 1;
                val = RecordMoveHash(pJunqi,&pJunqi->paHash,p,val,1);
            }

            pJunqi->pEngine->pDebugMove[cnt-1] = p;

            u8 test1[4] = {0x08,0x0E,0x08,0x0F};
            u8 test2[4] = {0x08,0x08,0x08,0x0B};
            u8 test3[4] = {0x06,0x05,0x06,0x0B};
            u8 test4[4] = {0x0A,0x0B,0x08,0x0B};
            u8 test5[4] = {0x08,0x01,0x09,0x01};
            u8 test6[4] = {0x0E,0x07,0x0F,0x06};
//            if(pJunqi->nDepth==3 && cnt==1 && !memcmp((u8*)&p->move,test3,4) )
//            {
//                pJunqi->bDebug = 1;
//            }

//            if( pJunqi->nDepth==2 && cnt==2 )
//            {
//                //log_a("eat");
//                if( !memcmp(pEngine->pDebugMove[0],test4,4) &&
//                         pEngine->pDebugMove[0]->move.result==2 &&
//                         pEngine->pDebugMove[1]->move.result==4 &&
//                        //!memcmp(pEngine->pDebugMove[1],test2,4) &&
//                       // !memcmp(pEngine->pDebugMove[2],test3,4) &&
//                        !memcmp((u8*)&p->move,test3,4) )
//                {
//                    pJunqi->bDebug = 1;
////                    log_a("cnt %d val %d per %d",cnt,val,p->percent);
////                    SafeMemout((u8*)&p->move, sizeof(p->move));
////                    sleep(1);
////                    assert(0);
//                }
//            }

            if( p->move.result==MOVE )
            {
                if( IsSearchAllMove(pJunqi,cnt,pData->iDir) )
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


//            if( cnt==2  )
//            {
//                //log_a("eat");
//
//                if( !memcmp(pEngine->pDebugMove[0],test4,4) )
////                     &&  !memcmp(pEngine->pDebugMove[1],test5,4) )
////                       && pEngine->pDebugMove[0]->move.result==2 )
////                if( !memcmp(pEngine->pDebugMove[0],test1,4)
////                        && !memcmp(pEngine->pDebugMove[1],test2,4)
////                        && !memcmp(pEngine->pDebugMove[2],test3,4) )
////                        && !memcmp(pEngine->pDebugMove[3],test4,4) )
//                {
//
//                    //if( pEngine->pDebugMove[0]->move.result==2 )
//                    {
//
//                        log_a("cnt %d val %d per %d",cnt,val,p->percent);
//                        //log_a("max %d",pData->mxVal);
//                        SafeMemout((u8*)&p->move, sizeof(p->move));
//                            static int jj=0;
//                            jj++;
////                            if(jj==11)
////                            {
////                                sleep(1);
////                            assert(0);
////                            }
//                            log_a("jj %d",jj);
//                    }
//                }
//            }



            //把局面撤回到上一步
            pJunqi->pMoveList = pData->pHead;
            assert(pJunqi->pEngine->pPos!=NULL);
            UnMakeMove(pJunqi,&p->move);

            if( p->move.result==EAT )
            {
                FreeMoveHashNode(pJunqi,p);
            }
        }


        if( preventFlag )
        {
            pJunqi->gFlag[FLAG_PREVENT] = 0;
        }
        if( p->move.result!=MOVE )
        {
            sum += val*p->percent;
            //log_a("sum %d val %d per %d",sum,val,p->percent);
            pRslt = &p->pNext->move;
            //下一个着法
            if( memcmp(&p->move, pRslt, 4) || p->pNext->isHead  )
            {
                pNextMove = p->pNext;
                aBestMove[cnt].mxPerFlag1 = 1;
                val = sum>>8;
                //这段代码起负作用
//                if( (pData->iDir&1)!=ENGINE_DIR && !pData->isGongB] )
//                {
//                    val = RecordMoveHash(pJunqi,&pJunqi->paHash,p,val,0);
//                }

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
                if( p->pNext->percent>mxPercent && p->move.result!=KILLED )
                {
                    pNextMove = p->pNext;
                    aBestMove[cnt].mxPerFlag1 = 1;
                }
                else
                {
                    aBestMove[cnt].mxPerFlag1 = 0;
                }
                goto continue_search;
            }

        }

        if( pJunqi->gFlag[TIME_OUT] )
        {
            pData->cut = 1;
            break;
        }


        if( cnt==1 && pJunqi->nDepth<4 )
        {
            AddMoveSortList(pJunqi,aBestMove,p,pMaxMove,val,0);
        }

        //产生截断
        if( val>=beta )//todo 被剪枝的最后可能分数不准确
        {
            if( -INFINITY==pData->mxVal && aBestMove[cnt-1].mxPerFlag1 &&
                    !pData->isGongB)
            {
                UpdateBestMove(pJunqi,aBestMove,p,pMaxMove,depth,cnt);
                memcpy(&pData->bestMove.move, &p->move, sizeof(p->move));

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
            pData->mxVal = val;
            //不要把模拟工兵的行棋更新到最佳路径里,否则之后的搜索会乱套
            if( aBestMove[cnt-1].mxPerFlag1 && !pData->isGongB )
            {
//                UpdateBestMove(pJunqi,aBestMove,pTemp,depth,cnt);
//                pData->pBest = &pTemp->move;
                UpdateBestMove(pJunqi,aBestMove,p,pMaxMove,depth,cnt);
                memcpy(&pData->bestMove.move, &p->move, sizeof(p->move));

                if( cnt==1 )
                {
                    if( pJunqi->nDepth>=4 )
                    {
                        AddMoveSortList(pJunqi,aBestMove,p,pMaxMove,val,0);
                    }
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


        //时间结束或收到go指令结束搜索
        if( TimeOut(pJunqi) )
        {
            //log_a("*****break**********");
            pData->cut = 1;
            pJunqi->gFlag[TIME_OUT] = 1;
        }

        if( p->pNext->isHead )
        {
            break;
        }


    }

    pData->pCur = p;

}


void SearchRailPath1(
        Junqi* pJunqi,
        BoardGraph *pSrc,
        BoardGraph *pDst,
        AlphaBetaData *pData )
{
    AdjNode *p;
    BoardGraph *pVertex;
    BoardChess *pChess = pSrc->pAdjList->pChess;


    pDst->cnt[pData->cnt]++;

    for(p=pDst->pAdjList->pNext; p!=NULL; p=p->pNext)
    {
        if( pData->cut )
        {
            break;
        }
        pVertex = &pJunqi->aBoard[p->pChess->point.x][p->pChess->point.y];


        if( pVertex->cnt[pData->cnt]!=0 )
        {
            continue;
        }
        //IsDirectRail必须要执行，所以放在前面
        else if( !IsDirectRail(pJunqi, pSrc, pDst, pVertex) && pChess->type!=GONGB )
        {
            continue;
        }
        else if( p->pChess->type!=NONE )
        {
            if( (p->pChess->pLineup->iDir&1)!=(pChess->pLineup->iDir&1) )
            {

                AddMoveToList(pJunqi, pChess, p->pChess, pData);
                pVertex->cnt[pData->cnt]++;
                //log_a("dir %d %d",p->pChess->iDir,pChess->iDir);
//                log_a("dst %d %d %d %d",pChess->point.x,pChess->point.y,
//                        p->pChess->point.x,p->pChess->point.y);

                SearchAlphaBeta(pJunqi,pData);
            }
            continue;
        }
        else
        {
            AddMoveToList(pJunqi, pChess, p->pChess, pData);
            //log_a("path %d %d %d %d",pChess->point.x,pChess->point.y,
                   // p->pChess->point.x,p->pChess->point.y);
            SearchAlphaBeta(pJunqi,pData);
            SearchRailPath1(pJunqi, pSrc, pVertex,pData);
        }
    }
}


void ClearDepthCnt(Junqi *pJunqi,int depth,u8 isClearCnt)
{
    int i,j;

    for(i=0; i<17; i++)
    {
        for(j=0; j<17; j++)
        {
            if( isClearCnt )
            {
                pJunqi->aBoard[i][j].cnt[depth] = 0;
            }
            else
            {
                pJunqi->aBoard[i][j].isSapperPath[depth] = 0;
            }
        }
    }
}

void SearchMoveList(
        Junqi* pJunqi,
        BoardChess *pSrc,
        AlphaBetaData *pData)
{

    BoardGraph *pVertex;
    BoardChess *pNbr;
    BoardGraph *pNbrVertex;

    int i,x,y;
    //log_c("clear");
    ClearDepthCnt(pJunqi,pData->cnt,1);
    ClearDepthCnt(pJunqi,pData->cnt,0);
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
        SearchRailPath1(pJunqi, pVertex, pVertex, pData);

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
            if( pNbrVertex->cnt[pData->cnt]!=0 )
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
                AddMoveToList(pJunqi, pSrc, pNbr,pData);
//                log_a("nbr1 %d %d %d %d",pSrc->point.x,pSrc->point.y,
//                        pNbr->point.x,pNbr->point.y);

                SearchAlphaBeta(pJunqi,pData);
            }
            //非斜相邻
            else if( pNbr->point.x==pSrc->point.x || pNbr->point.y==pSrc->point.y)
            {
                AddMoveToList(pJunqi, pSrc, pNbr,pData);
//                log_a("nbr2 %d %d %d %d",pSrc->point.x,pSrc->point.y,
//                        pNbr->point.x,pNbr->point.y);
                SearchAlphaBeta(pJunqi,pData);
            }

        }
    }

}

u8 SelectSearchType(Junqi *pJunqi, int myTurn, int iDir)
{
    u8 rc = 0;
    SearchType type;

    if( pJunqi->eSearchType==SEARCH_DEEP )
    {
        type = pJunqi->eDeepType;
    }
    else
    {
        type = pJunqi->eSearchType;
    }

    switch( type )
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

//mx_type没有入栈，所以搜索完毕要重新调整一下
void ReAdjustMaxType(Junqi *pJunqi)
{
    int i;

    i = (ENGINE_DIR+1)%4;
    if( !pJunqi->aInfo[i].bDead )
    {
        AdjustMaxType(pJunqi,i);
    }
    i = (ENGINE_DIR+3)%4;
    if( !pJunqi->aInfo[i].bDead )
    {
        AdjustMaxType(pJunqi,i);
    }
}

u8 SearchEat(Junqi *pJunqi)
{
    Engine *pEngine = pJunqi->pEngine;
    u8 maxCnt;

    //todo 这里更应该用nLive来比较
    if( pJunqi->nEat<10 )
    {
        maxCnt = pJunqi->nDepth;
    }
    if( pJunqi->nEat<20 )
    {
        maxCnt = pJunqi->nDepth+1;
    }
    else if(pJunqi->nEat<40 )
    {
        maxCnt = pJunqi->nDepth+2;
    }
    else
    {
        //层数太多在cmd中运行会出现内存出错现象，而在eclipse中运行却无法复现
        maxCnt = pJunqi->nDepth+3;
    }

    if( pEngine->eatIndex>4 || pJunqi->cnt>maxCnt )
    {
        return 0;
    }

    return pEngine->eatInList;
}

u8 HasNoBestMove(Junqi *pJunqi, int value, int iDir)
{
    int val;
    u8 rc = 0;

    val = EvalSituation(pJunqi,0);
    if( (iDir&1)!=ENGINE_DIR )
    {
        val = -val;
    }

    if( val>value )
    {
        rc = 1;
    }

    return rc;

}
int AlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        u8 isMove )
{
    int i;
    BoardChess *pSrc;
    ChessLineup *pLineup;
    int val;
    int temp[3] = {0};
    ChessLineup tempLineup;
    u8 flag = 0;
    u8 eatFlag = 0;
    int iDir = pJunqi->eTurn;
    AlphaBetaData search_data;
    Engine *pEngine = pJunqi->pEngine;
    BestMove *aBestMove = pJunqi->pEngine->aBestMove;

    if( !pJunqi->cnt )
    {
        pJunqi->paHash = NULL;
        pJunqi->nDepth =depth;
        aBestMove[0].mxPerFlag = 1;
        aBestMove[0].mxPerFlag1 = 1;
        aBestMove[0].pNode = aBestMove[0].pHead;
        memset(pJunqi->gFlag,0,sizeof(pJunqi->gFlag));
        pEngine->eatInList = 0;
        pEngine->eatIndex = 0;
        pJunqi->myTurn = iDir;
        if( pJunqi->eSearchType==SEARCH_DEEP )
        {
            pJunqi->myTurn = pJunqi->deepTurn;
        }
    }
    pJunqi->cnt++;


    pEngine->eatIndex++;
    if( pJunqi->gFlag[FLAG_EAT] )
    {

        eatFlag = 1;
        depth = 0;
        if( !SearchEat(pJunqi)  )//最多搜到30层
        {
            pJunqi->gFlag[FLAG_EAT] = 0;
        }
    }



    memset(&search_data,0,sizeof(search_data));
    search_data.depth = depth;
    search_data.alpha = alpha;
    search_data.beta = beta;
    search_data.iDir = iDir;
    search_data.mxVal = -INFINITY;
    search_data.cnt = pJunqi->cnt;

    //遍历到最后一层时计算局面分值
    if( depth==0 && !pJunqi->gFlag[FLAG_EAT] )
    {
#if 1

#if 1
        if( pJunqi->gFlag[FLAG_PREVENT]==2 )//在阻挡搜索中不再搜索吃子
        {
            pEngine->eatInList = 0;
        }

       // if( 1==pJunqi->gFlag[FLAG_PREVENT] && pJunqi->eSearchType!=SEARCH_DEFAULT )
        if( 1==pJunqi->gFlag[FLAG_PREVENT] )
        {
            pJunqi->gFlag[FLAG_PREVENT] = 2;
            pJunqi->cnt--;
            val = CallAlphaBeta1(pJunqi,2,alpha,beta,iDir,isMove);
            pJunqi->cnt++;
            pJunqi->gFlag[FLAG_PREVENT] = 1;
            pJunqi->gFlag[FLAG_EAT] = eatFlag;
        }
//        else if( pJunqi->eSearchType==SEARCH_DEEP &&
//                SearchEat(pJunqi) )
        else if( SearchEat(pJunqi) )
#else
        if( SearchEat(pJunqi) )
#endif
        {
            pJunqi->gFlag[FLAG_EAT] = 1;
            pJunqi->cnt--;
            val = CallAlphaBeta1(pJunqi,depth,alpha,beta,iDir,isMove);
            pJunqi->cnt++;
            pJunqi->gFlag[FLAG_EAT] = 0;
        }
        else
        {

            val = EvalSituation(pJunqi,0);

//            if(val==290 && pJunqi->nDepth==3 && pJunqi->bDebug )
//            {
//                log_a("sss");
//
//                sleep(1);
//                assert(0);
//            }
            pJunqi->gFlag[FLAG_EAT] = eatFlag;
            //EvalSituation是针对引擎评价的，所以对方的分值应取负值
            if( (iDir&1)!=ENGINE_DIR )
            {
                val = -val;
            }
            if( aBestMove[pJunqi->cnt-1].pHead!=NULL )
            {
                aBestMove[pJunqi->cnt-1].pHead->isMove = 0;
            }
        }
#else
        val = EvalSituation(pJunqi,0);
        if( iDir%2!=ENGINE_DIR%2 )
        {
            val = -val;
        }
#endif

       // val = EvalSituation(pJunqi);

        pJunqi->test_num++;
        //val = 5;

        pJunqi->cnt--;
        //log_a("test num %d", pJunqi->test_num);
        return val;
    }

    //2方单独测试
    if( SelectSearchType(pJunqi,pJunqi->myTurn,iDir) )
    {
        ChessTurn(pJunqi);
        pJunqi->cnt--;

        val = CallAlphaBeta1(pJunqi,depth,alpha,beta,iDir,isMove);

        //下面这段代码不是重复了吗，当cnt是1时，那么在CallAlphaBeta1里递归会变为1
        //在那里会做各种释放工作，不知道当时怎么想的，测试没出现问题先留着
        //可以看到这段代码是无效的，search_data并没有使用到
        if( 0==pJunqi->cnt )
        {
           // log_c("jj %d",kk);
            pJunqi->cnt = 0;
            SetBestMove(pJunqi,&search_data.bestMove.move);
        }
        ClearMoveList(pJunqi,search_data.pHead);
        if( search_data.hasBest )
        {
            FreeMoveHashNode(pJunqi,&search_data.bestMove);
        }

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
        SearchMoveList(pJunqi,pSrc,&search_data);

        if( pSrc->pLineup->type==DARK && pSrc->isRailway &&
            pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB]<3 && !flag
            && !pJunqi->gFlag[FLAG_EAT] && !pJunqi->gFlag[FLAG_PREVENT]  )
        {
            temp[0] = pSrc->type;
            memcpy(&tempLineup,pSrc->pLineup,sizeof(tempLineup));
            pSrc->type = GONGB;
            pSrc->pLineup->type = GONGB;
            pSrc->pLineup->mx_type = GONGB;
            pSrc->pLineup->isNotBomb = 1;
            pSrc->pLineup->isNotLand = 1;
            pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB]++;
            pJunqi->aInfo[pSrc->pLineup->iDir].aLiveTypeSum[GONGB]++;
            search_data.isGongB = 1;
            SearchMoveList(pJunqi,pSrc,&search_data);
            search_data.isGongB = 0;
            pJunqi->aInfo[pSrc->pLineup->iDir].aTypeNum[GONGB]--;
            pJunqi->aInfo[pSrc->pLineup->iDir].aLiveTypeSum[GONGB]--;
            memcpy(pSrc->pLineup,&tempLineup,sizeof(tempLineup));
            pSrc->type = temp[0];
            if( 2==search_data.isGongB )
            {
                flag = 1;
            }
        }

        val = search_data.mxVal;
    }

//    if( NULL==pJunqi->pMoveList ||
//            ( pJunqi->gFlag[FLAG_EAT] && !search_data.hasEat ) )
    if( NULL==pJunqi->pMoveList ||
            ( pJunqi->gFlag[FLAG_EAT] && HasNoBestMove(pJunqi,val,iDir) ) )
    {
        int temp;
        pJunqi->eTurn = iDir;
        ChessTurn(pJunqi);
        temp = val;
        pJunqi->cnt--;
      //  log_a("%d alpha %d beta %d",pJunqi->cnt,alpha,beta);
        val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,iDir,isMove);//todo 最后一个参数不能固定1
        pJunqi->cnt++;

        if( NULL!=pJunqi->pMoveList )
        {
            if( temp>val )
            {
                val = temp;
            }
        }


        //aBestMove[pJunqi->cnt-1].flag2 = 0;
        if( aBestMove[pJunqi->cnt-1].pHead!=NULL )
        {
            aBestMove[pJunqi->cnt-1].pHead->isMove = 0;
        }
        //在SearchBestMove里，一层可能有多种碰撞结果
        //不是概率最大的一种，不要修改pNode
        //因为best的搜索只在概率最大的分支下面
        if( aBestMove[pJunqi->cnt-1].mxPerFlag )
        {
            aBestMove[0].pNode = NULL;
        }
    }


    if( 1==pJunqi->cnt )
    {
        //pJunqi->cnt = 0;
        ReAdjustMaxType(pJunqi);
        //log_a("get best");
        SetBestMove(pJunqi,&search_data.bestMove.move);
        ClearMoveHash(pJunqi,&pJunqi->paHash);
        //只在单线程有效
        log_a("malloc %d free %d",malloc_cnt,free_cnt);

    }
    ClearMoveList(pJunqi,search_data.pHead);
    if( search_data.hasBest )
    {
        FreeMoveHashNode(pJunqi,&search_data.bestMove);
    }
    pJunqi->cnt--;

    return val;
}
