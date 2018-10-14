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
    MoveResultData *pBest;
    MoveList *pCur;
    MoveList *pHead;
    int iDir;
    u8 cut;
}AlphaBetaData;


int CallAlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir )
{
    int val;

    if( iDir%2==pJunqi->eTurn%2 )
    {
        //下家阵亡轮到对家走
        val = AlphaBeta1(pJunqi,depth,alpha,beta);
    }
    else
    {
        val = -AlphaBeta1(pJunqi,depth,-beta,-alpha);
    }

    return val;
}

u8 IsNotSameMove(MoveList *p)
{
    u8 rc = 0;
    if( !p->isHead )
    {
        rc = memcmp(&p->move, &p->pPre->move, 4);
    }

    return rc;
}

void SearchAlphaBeta(
        Junqi *pJunqi,
        AlphaBetaData *pData
        )
{
    int depth = pData->depth;
    int alpha = pData->alpha;
    int beta = pData->beta;
    MoveResultData *pRslt;
    MoveList *p;
    int val;
    int sum = 0;

    int iKey;

    if( pData->pCur==NULL )
    {
        pData->pCur = pJunqi->pMoveList;
        pData->pHead = pJunqi->pMoveList;
    }
    else
    {
        pData->pCur = pData->pCur->pNext;
    }

    for(p=pData->pCur; pData->pCur!=NULL; p=p->pNext)
    {
        pJunqi->eTurn = pData->iDir;
        //模拟着法产生后的局面
        //test(pJunqi,&p->move,pHead);
        MakeNextMove(pJunqi,&p->move);
        assert(pJunqi->pEngine->pPos!=NULL);
        iKey = GetHashKey(pJunqi);
        //log_a("key %d",iKey);
        if( CheckMoveHash(&pJunqi->paHash,iKey) &&
                IsNotSameMove(p) )
        {
            if( p->move.result>MOVE )
            {
                while( !memcmp( &p->move, &p->pNext->move, 4) )
                {
                    if( p->pNext->isHead ) break;

                    p = p->pNext;
                }
            }
            UnMakeMove(pJunqi,&p->move);
            goto continue_search;
        }
        else
        {
            val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,pData->iDir);

//            log_a("depth %d val %d per %d",depth,val,p->percent);
//            SafeMemout((u8*)&p->move, sizeof(p->move));

            //把局面撤回到上一步
            pJunqi->pMoveList = pData->pHead;
            assert(pJunqi->pEngine->pPos!=NULL);
            UnMakeMove(pJunqi,&p->move);
        }

        if( p->move.result!=MOVE )
        {
            sum += val*p->percent;

            pRslt = &p->pNext->move;
            //下一个着法
            if( memcmp(&p->move, pRslt, 4) || p->pNext->isHead )
            {
                val = sum>>8;
                sum = 0;
            }
            else
            {
                goto continue_search;
            }
        }
        //产生截断
        if( val>=beta )
        {
            pData->alpha = val;
            pData->cut = 1;
            break;
        }
        //更新alpha值
        if( val>alpha )
        {
            pData->pBest = &p->move;
            pData->alpha = val;
        }

    continue_search:

        if( p->pNext->isHead )
        {
            break;
        }
        //时间结束或收到go指令结束搜索
        if( TimeOut(pJunqi) )
        {
            pData->cut = 1;
            break;
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
        else if( pChess->type!=GONGB && !IsDirectRail(pJunqi, pSrc, pVertex) )
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

    search_data.depth = depth;
    search_data.alpha = alpha;
    search_data.beta = beta;
    search_data.cut = 0;
    search_data.pBest = NULL;
    search_data.iDir = iDir;
    search_data.pCur = NULL;
    search_data.pHead = NULL;


    if( !cnt )
    {
        pJunqi->paHash = NULL;
    }
    cnt++;
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
        val = search_data.alpha;
    }

    if( NULL==pJunqi->pMoveList )
    {
        pJunqi->eTurn = iDir;
        ChessTurn(pJunqi);

        val = CallAlphaBeta1(pJunqi,depth-1,alpha,beta,iDir);
    }

    cnt--;
    if( 0==cnt )
    {
        cnt = 0;
        SetBestMove(pJunqi,search_data.pBest);
        ClearMoveHash(&pJunqi->paHash);
    }
    ClearMoveList(search_data.pHead);

    return val;
}
