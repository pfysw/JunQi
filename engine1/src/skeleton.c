/*
 * skeleton.c
 *
 *  Created on: Nov 27, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include "comm.h"
#include "skeleton.h"

Skeleton *SkeletonOpen(void)
{
    Skeleton *pSkeleton = (Skeleton*)malloc(sizeof(Skeleton));
    memset(pSkeleton, 0, sizeof(Skeleton));

    return pSkeleton;
}
