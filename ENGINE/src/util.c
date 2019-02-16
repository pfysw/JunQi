/*
 * util.c
 *
 *  Created on: Dec 29, 2018
 *      Author: Administrator
 */

#include"junqi.h"

//放一些基础算法


static MoveSort *MergeMoveValueList(MoveSort *pA, MoveSort *pB, int type, int depth){
   MoveSort result, *pTail;
  pTail = &result;
  assert( pA!=0 && pB!=0 );
  for(;;){
    if( pA->aValue[depth][type]>pB->aValue[depth][type] ){
      pTail->pNext = pA;
      pTail = pA;
      pA = pA->pNext;
      if( pA==0 ){
        pTail->pNext = pB;
        break;
      }
    }else{
      pTail->pNext = pB;
      pTail = pB;
      pB = pB->pNext;
      if( pB==0 ){
        pTail->pNext = pA;
        break;
      }
    }
  }
  return result.pNext;
}

#define N_SORT_BUCKET  32
MoveSort *SortMoveValueList(MoveSort *pIn, int type, int depth){
  MoveSort *a[N_SORT_BUCKET], *p;
  int i;
  memset(a, 0, sizeof(a));
  while( pIn ){

    p = pIn;
    pIn = p->pNext;
    p->pNext = 0;
    for(i=0; ALWAYS(i<N_SORT_BUCKET-1); i++){
      if( a[i]==0 ){
        a[i] = p;
        break;
      }else{

        p = MergeMoveValueList(a[i], p, type, depth);

        a[i] = 0;
      }
    }
    if( NEVER(i==N_SORT_BUCKET-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = MergeMoveValueList(a[i], p, type, depth);
    }
  }
  p = a[0];
  for(i=1; i<N_SORT_BUCKET; i++){
    if( a[i]==0 ) continue;
    p = p ? MergeMoveValueList(a[i], p, type, depth) : a[i];
  }
  return p;
}
