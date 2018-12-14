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
    u8 adjusrDir;
    u8 adjusrIndex;
    u8 saveType;

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
	ExtraAdjustInfo xExtraInfo;
};

typedef struct MoveHash MoveHash;
struct MoveHash
{
    int iKey;
    u8 depth;
    u8 iDir;
    int value;
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
void AddMoveToHash(
        Junqi *pJunqi,
        BoardChess *pSrc,
        BoardChess *pDst );
u8 IsNotSameMove(MoveList *p);

int SearchBestMove(
        Junqi *pJunqi,
        BestMove *aBestMove,
        int cnt,
        int alpha,
        int beta,
        MoveResultData **ppBest ,
        int depth,
        int flag
        );
int AlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta);

void FreeBestMoveList(
        BestMove *aBeasMove,
        int depth);

int CallAlphaBeta1(
        Junqi *pJunqi,
        int depth,
        int alpha,
        int beta,
        int iDir,
        u8 isMove);
void RecordMoveHash(
        MoveHash ***paHash,
        int iKey,
        u8 iDir,
        u8 depth,
        int val);

void UpdateBestMove(
        BestMove *aBeasMove,
        MoveList *pMove,
        int depth,
        int cnt,
        u8 isHashVal);
int CheckMoveHash(MoveHash ***paHash, int iKey, int depth, int iDir);
int GetHashKey(Junqi* pJunqi);
void MakeNextMove(Junqi *pJunqi, MoveResultData *pResult);
void UnMakeMove(Junqi *pJunqi, MoveResultData *pResult);
void SetBestMove(Junqi *pJunqi, MoveResultData *pResult);
void PrintBestMove(BestMove *aBestMove, int alpha, int depth);
void UpdatePathValue(
        Junqi *pJunqi,
        BestMove *aBestMove,
        int iDir,
        int cnt );
#endif /* SEARCH_H_ */
