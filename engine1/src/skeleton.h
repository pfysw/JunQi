/*
 * skeleton.h
 *
 *  Created on: Nov 27, 2021
 *      Author: Administrator
 */

#ifndef SKELETON_H_
#define SKELETON_H_
#include "jtype.h"

enum State1
{
    SKL_COMM_INIT,
    SKL_COMM_READY
};

enum State2
{
    SKL_COMM
};

enum SklPrintType
{
    SKL_LINEUP,
    SKL_POS_PROP,
    SKL_POS_POINT
};

typedef struct SklState
{
    enum State1  eState;
    enum State2  eType;
}SklState;

typedef struct Skeleton Skeleton;
struct Skeleton
{
    SklState *pCommState;
    Junqi *pJunqi;
};

Skeleton *SkeletonOpen(Junqi *pJunqi);
void CheckChessInit(Skeleton *pSkl);

#endif /* SKELETON_H_ */
