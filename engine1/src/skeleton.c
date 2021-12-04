/*
 * skeleton.c
 *
 *  Created on: Nov 27, 2021
 *      Author: Administrator
 */


Skeleton *SkeletonOpen(void)
{
    Skeleton *pSkeleton = (Skeleton*)malloc(sizeof(Skeleton));
    memset(pSkeleton, 0, sizeof(Skeleton));

    return pSkeleton;
}
