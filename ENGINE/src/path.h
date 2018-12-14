/*
 * path.h
 *
 *  Created on: Sep 4, 2018
 *      Author: Administrator
 */

#ifndef PATH_H_
#define PATH_H_



int IsEnableMove(Junqi *pJunqi, BoardChess *pSrc, BoardChess *pDst);
u8 CanMovetoJunqi(Engine *pEngine, BoardChess *pSrc);
void ClearPathCnt(Junqi *pJunqi);
void ClearPassCnt(Junqi *pJunqi);
void GenJunqiPath(
        Junqi *pJunqi,
        JunqiPathList *pSrcData,
        int isLeft);
void GetJunqiPath(
        Engine *pEngine,
        JunqiPathList *pSrcData );
void InitJunqiPath(Junqi *pJunqi);
void ClearJunqiPath(Engine *pEngine, int iPath);
int GetJunqiPathValue(Junqi *pJunqi, int iDir);

#endif /* PATH_H_ */
