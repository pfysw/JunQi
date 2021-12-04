/*
 * skeleton.h
 *
 *  Created on: Nov 27, 2021
 *      Author: Administrator
 */

#ifndef SKELETON_H_
#define SKELETON_H_

enum State1
{
    SKL_COMM_INIT,
    SKL_COMM_READY
};

enum State2
{
    SKL_COMM
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
};

#endif /* SKELETON_H_ */
