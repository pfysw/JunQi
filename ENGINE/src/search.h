/*
 * search.h
 *
 *  Created on: Sep 20, 2018
 *      Author: Administrator
 */

#ifndef SEARCH_H_
#define SEARCH_H_

//#include "junqi.h"
#include "type.h"

typedef struct ExtraAdjustInfo
{
    u8 adjustFlag;
    ChessLineup xLineup;

}ExtraAdjustInfo;

typedef struct PositionData PositionData;
struct PositionData
{
	BoardChess xSrcChess;
	BoardChess xDstChess;
	ChessLineup xSrcLineup;
	ChessLineup xDstLineup;
	ChessLineup xJunqiLineup[2];
	u8 mx_type[30];
	PartyInfo info[2];
	u8 junqi_chess_type[2];
	u8 isSrcdead;
	u8 isDstDead;
	u8 eatInList;
	u8 eatIndex;
	ExtraAdjustInfo xExtraInfo;
};

typedef struct AlphaBetaData AlphaBetaData;
struct AlphaBetaData
{
    int alpha;
    int beta;
    int depth;
    u8 aInitBest[4];
    MoveList bestMove;
    MoveList *pCur;
    MoveList *pHead;
    int mxVal;
    int iDir;
    u8 cnt;
    u8 hasBest;
    u8 hasEat;
    u8 cut;
    u8 bestFlag;
    u8 isGongB;
};

typedef struct MoveHash MoveHash;
struct MoveHash
{
    int iKey;
    MoveList *pMove;
    u8 cnt;
    u8 eatFlag;
    u8 iDir;
    u8 bSet;
//    int value;
    MoveHash *pNext;
};


struct PositionList
{
	PositionData data;
	PositionList *pNext;
	PositionList *pPre;
	int value;
	u8 isHead;
};

void PopMoveFromStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove );

void PushMoveToStack(
	Junqi *pJunqi,
	BoardChess *pSrc,
	BoardChess *pDst,
	MoveResultData *pMove );


int AlphaBeta(
		Junqi *pJunqi,
		int depth,
		int alpha,
		int beta);
int TimeOut(Junqi *pJunqi);
u8 SendBestMove(Engine *pEngine);
void ClearMoveHash(Junqi *pJunqi,MoveHash ***paHash);
int AddMoveToHash(
        Junqi *pJunqi,
        MoveList *pMove,
        u8 isEat);

u8 IsNotSameMove(MoveList *p);

int SearchBestMove(
        Junqi *pJunqi,
        BestMove *aBestMove,
        AlphaBetaData *pData,
        MoveList *pBest ,
        int flag
        );
int AlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        u8 isMove);

void FreeBestMoveList(
        Junqi *pJunqi,
        BestMove *aBeasMove,
        int depth);

int CallAlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir,
        u8 isMove);

int RecordMoveHash(
        Junqi *pJunqi,
        MoveHash ***paHash,
        MoveList *pMove,
        int alpha,
        u8 isEat );

void UpdateBestMove(
        Junqi *pJunqi,
        BestMove *aBeasMove,
        MoveList *pMove,
        MoveList *pMaxMove,
        int depth,
        int cnt);

void UpdateBestList(
        Junqi *pJunqi,
        BestMoveList *pDst,
        BestMoveList *pSrc,
        u8 isShare);
int CheckMoveHash(
        Junqi *pJunqi,
        BoardChess *pDst);
int GetHashKey(Junqi* pJunqi);
void MakeNextMove(Junqi *pJunqi, MoveResultData *pResult,u8 *flag);
void UnMakeMove(Junqi *pJunqi, MoveResultData *pResult);
void SetBestMove(Junqi *pJunqi, MoveResultData *pResult);
void PrintBestMove(BestMove *aBestMove, int alpha);
void UpdatePathValue(
        Junqi *pJunqi,
        BestMove *aBestMove,
        int iDir,
        int cnt );
void ClearMoveSortList(Junqi *pJunqi);
void FreeSortMoveNode(Junqi *pJunqi, BestMoveList *pNode);
void AddMoveSortList(
        Junqi *pJunqi,
        BestMove *aBestMove,
        void *pSrc,
        MoveList *pMaxMove,
        int value,
        u8 flag);
void FreeMoveHashNode(Junqi *pJunqi,  MoveList *pMove);
void PrintMoveSortList(Junqi *pJunqi);
int SelectSortMove(Junqi *pJunqi);
int GetMaxPerMove(MoveResult *result);
int ProSearch(Junqi* pJunqi,int depth);
void SetBestMoveNode(
        BestMoveList *pList,
        MoveList *pMove,
        MoveList *pMaxMove );
void SetPathValue(Junqi *pJunqi);
void FindBestPathMove(Junqi *pJunqi);
void ReAdjustMaxType(Junqi *pJunqi);
void FindBestMove(
        Junqi *pJunqi,
        MoveSort *pHead,
        MoveSort **pResult,
        int type,
        int depth,
        u8 hasHead );
int DeepSearch(Junqi *pJunqi, BestMoveList *pNode, int type, int depth);
MoveSort *VerifyDeepMove(Junqi *pJunqi, MoveSort *pHead);
void CheckPreventMove(
        Junqi *pJunqi,
        MoveList *p,
        u8 *preventFlag);
#endif /* SEARCH_H_ */
