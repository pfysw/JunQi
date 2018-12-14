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
//后来测试发现GenerateMoveList占用的时间很少
//主要时间耗在了CheckMoveHash里，所以性能提升有限

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
            pData->alpha = pData->mxVal;
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
            if( p->move.result==MOVE )
            {
                val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,1);
            }
            else
            {
                //碰撞中有3种情况，不能截断
                val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir,0);
            }
#ifdef MOVE_HASH
            RecordMoveHash(&pJunqi->paHash,iKey,pJunqi->eTurn,depth,val);
#endif

//            if(cnt<2)
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

        //产生截断
        if( val>=beta )
        {
            if( -INFINITY==pData->mxVal && aBestMove[cnt-1].mxPerFlag1 )
            {
                UpdateBestMove(aBestMove,p,depth,cnt,isHashVal);
                UpdatePathValue(pJunqi,aBestMove,pData->iDir,cnt);
                pData->pBest = &p->move;
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val,depth);
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
                UpdateBestMove(aBestMove,p,depth,cnt,isHashVal);
                pData->pBest = &p->move;
                if( cnt==1 )
                {
                    PrintBestMove(aBestMove,val,depth);
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
    static int cnt = 0;
    int iDir = pJunqi->eTurn;
    AlphaBetaData search_data;
    BestMove *aBestMove = pJunqi->pEngine->aBestMove;

    if( !cnt )
    {
        pJunqi->paHash = NULL;
        aBestMove[0].mxPerFlag = 1;
        aBestMove[0].mxPerFlag1 = 1;
        aBestMove[0].pNode = aBestMove[0].pHead;
    }
    cnt++;

    search_data.depth = depth;
    search_data.alpha = alpha;
    search_data.beta = beta;
    search_data.cut = 0;
    search_data.pBest = NULL;
    search_data.iDir = iDir;
    search_data.pCur = NULL;
    search_data.pHead = NULL;
    search_data.mxVal = -INFINITY;
    search_data.cnt = cnt;
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
        cnt--;
        //log_a("test num %d", pJunqi->test_num);
        return val;
    }

    //2方单独测试
//    if( iDir==0||iDir==1)
//    {
//        ChessTurn(pJunqi);
//        cnt--;
//        val = CallAlphaBeta1(pJunqi,depth,alpha,beta,iDir,1);
//
//        if( 0==cnt )
//        {
//            cnt = 0;
//            SetBestMove(pJunqi,search_data.pBest);
//        }
//        ClearMoveList(search_data.pHead);
//
//        return val;
//    }


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
        aBestMove[cnt-1].flag2 = 0;
        //不要再搜下一层了
        //例如3层搜索时，刚开始是最好着法是3步棋
        //后来搜到2步棋的最好着法，第3步无棋可走
        //由于代码原因，在链表里的第3步仍然还在
        //第4层搜索时不能把这一步搜进去
        //*********************************
        //在SearchBestMove里，一层可能有多种碰撞结果
        //不是概率最大的一种，不要修改pNode
        //因为best的搜索只在概率最大的分支下面
        if( aBestMove[cnt-1].mxPerFlag )
        {
            aBestMove[0].pNode = NULL;
        }
    }

    cnt--;
    if( 0==cnt )
    {
        cnt = 0;
        //log_a("get best");
        SetBestMove(pJunqi,search_data.pBest);
#ifdef MOVE_HASH
        ClearMoveHash(&pJunqi->paHash);
#endif
    }
    ClearMoveList(search_data.pHead);

    return val;
}
