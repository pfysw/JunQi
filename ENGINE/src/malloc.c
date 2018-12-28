/*
 * malloc.c
 *
 *  Created on: Dec 23, 2018
 *      Author: Administrator
 */
#include "junqi.h"

//代码取自Sqlite3的mem5.c
//采用的是buddy内存分配算法
//每个线程都有单独的内存池来管理内存
//之所以不用malloc是因为多线程下，malloc的代码会加锁
//无法有效利用多核cpu的性能

typedef struct Mem5Link Mem5Link;
struct Mem5Link {
  int next;       /* Index of next free chunk */
  int prev;       /* Index of previous free chunk */
};

/*
** Maximum size of any allocation is ((1<<LOGMAX)*pMem->szAtom). Since
** pMem->szAtom is always at least 8 and 32-bit integers are used,
** it is not actually possible to reach this limit.
*/
#define LOGMAX 30

/*
** Masks used for pMem->aCtrl[] elements.
*/
#define CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#define CTRL_FREE     0x20    /* True if not checked out */

/*
** All of the static variables used by this module are collected
** into a single structure named "mem5".  This is to keep the
** static variables organized and to reduce namespace pollution
** when this module is combined with other in the amalgamation.
*/
typedef struct Mem5Global {
  /*
  ** Memory available for allocation
  */
  int szAtom;      /* Smallest possible allocation in bytes */
  int nBlock;      /* Number of szAtom sized blocks in zPool */
  u8 *zPool;       /* Memory available to be allocated */

  /*
  ** Lists of free blocks.  aiFreelist[0] is a list of free blocks of
  ** size pMem->szAtom.  aiFreelist[1] holds blocks of size szAtom*2.
  ** aiFreelist[2] holds free blocks of size szAtom*4.  And so forth.
  */
  int aiFreelist[LOGMAX+1];

  /*
  ** Space for tracking which blocks are checked out and the size
  ** of each block.  One byte per block.
  */
  u8 *aCtrl;

} Mem5Global;

/*
** Assuming pMem->zPool is divided up into an array of Mem5Link
** structures, return a pointer to the idx-th such link.
*/
#define MEM5LINK(idx) ((Mem5Link *)(&pMem->zPool[(idx)*pMem->szAtom]))

/*
** Unlink the chunk at pMem->aPool[i] from list it is currently
** on.  It should be found on pMem->aiFreelist[iLogsize].
*/
static void memsys5Unlink(Mem5Global *pMem, int i, int iLogsize){
  int next, prev;
  assert( i>=0 && i<pMem->nBlock );
  assert( iLogsize>=0 && iLogsize<=LOGMAX );
  assert( (pMem->aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  next = MEM5LINK(i)->next;
  prev = MEM5LINK(i)->prev;
  if( prev<0 ){
    pMem->aiFreelist[iLogsize] = next;
  }else{
    MEM5LINK(prev)->next = next;
  }
  if( next>=0 ){
    MEM5LINK(next)->prev = prev;
  }
}

/*
** Link the chunk at pMem->aPool[i] so that is on the iLogsize
** free list.
*/
static void memsys5Link(Mem5Global *pMem, int i, int iLogsize){
  int x;
  assert( i>=0 && i<pMem->nBlock );
  assert( iLogsize>=0 && iLogsize<=LOGMAX );
  assert( (pMem->aCtrl[i] & CTRL_LOGSIZE)==iLogsize );

  x = MEM5LINK(i)->next = pMem->aiFreelist[iLogsize];
  MEM5LINK(i)->prev = -1;
  if( x>=0 ){
    assert( x<pMem->nBlock );
    MEM5LINK(x)->prev = i;
  }
  pMem->aiFreelist[iLogsize] = i;
}

/*
** Return the size of an outstanding allocation, in bytes.
** This only works for chunks that are currently checked out.
*/
int memsys5Size(Mem5Global *pMem, void *p){
  int iSize, i;
  assert( p!=0 );
  i = (int)(((u8 *)p-pMem->zPool)/pMem->szAtom);
  assert( i>=0 && i<pMem->nBlock );
  iSize = pMem->szAtom * (1 << (pMem->aCtrl[i]&CTRL_LOGSIZE));
  return iSize;
}

/*
** Return a block of memory of at least nBytes in size.
** Return NULL if unable.  Return NULL if nBytes==0.
**
** The caller guarantees that nByte is positive.
**
** The caller has obtained a mutex prior to invoking this
** routine so there is never any chance that two or more
** threads can be in this routine at the same time.
*/
void *memsys5Malloc(Junqi *pJunqi, int nByte){
  int i;           /* Index of a pMem->aPool[] slot */
  int iBin;        /* Index into pMem->aiFreelist[] */
  int iFullSz;     /* Size of allocation rounded up to power of 2 */
  int iLogsize;    /* Log2 of iFullSz/POW2_MIN */

  Mem5Global *pMem = pJunqi->pThreadMem;

  /* nByte must be a positive */
  assert( nByte>0 );

  /* No more than 1GiB per allocation */
  if( nByte > 0x40000000 ) return 0;


  /* Round nByte up to the next valid power of two */
  for(iFullSz=pMem->szAtom,iLogsize=0; iFullSz<nByte; iFullSz*=2,iLogsize++){}

  /* Make sure pMem->aiFreelist[iLogsize] contains at least one free
  ** block.  If not, then split a block of the next larger power of
  ** two in order to create a new free block of size iLogsize.
  */
  for(iBin=iLogsize; iBin<=LOGMAX && pMem->aiFreelist[iBin]<0; iBin++){}
  if( iBin>LOGMAX ){
    assert(0);
  }
  i = pMem->aiFreelist[iBin];
  memsys5Unlink(pMem, i, iBin);
  while( iBin>iLogsize ){
    int newSize;

    iBin--;
    newSize = 1 << iBin;
    pMem->aCtrl[i+newSize] = CTRL_FREE | iBin;
    memsys5Link(pMem, i+newSize, iBin);
  }
  pMem->aCtrl[i] = iLogsize;

  pJunqi->malloc_cnt++;

  /* Return a pointer to the allocated memory. */
  return (void*)&pMem->zPool[i*pMem->szAtom];
}

/*
** Free an outstanding memory allocation.
*/
void memsys5Free(Junqi *pJunqi, void *pOld){
  u32 size, iLogsize;
  int iBlock;

  Mem5Global *pMem = pJunqi->pThreadMem;

  /* Set iBlock to the index of the block pointed to by pOld in
  ** the array of pMem->szAtom byte blocks pointed to by pMem->zPool.
  */
  iBlock = (int)(((u8 *)pOld-pMem->zPool)/pMem->szAtom);

  /* Check that the pointer pOld points to a valid, non-free block. */
  assert( iBlock>=0 && iBlock<pMem->nBlock );
  assert( ((u8 *)pOld-pMem->zPool)%pMem->szAtom==0 );
  assert( (pMem->aCtrl[iBlock] & CTRL_FREE)==0 );

  iLogsize = pMem->aCtrl[iBlock] & CTRL_LOGSIZE;
  size = 1<<iLogsize;
  assert( iBlock+size-1<(u32)pMem->nBlock );

  pMem->aCtrl[iBlock] |= CTRL_FREE;
  pMem->aCtrl[iBlock+size-1] |= CTRL_FREE;

  pMem->aCtrl[iBlock] = CTRL_FREE | iLogsize;
  while( ALWAYS(iLogsize<LOGMAX) ){
    int iBuddy;
    if( (iBlock>>iLogsize) & 1 ){
      iBuddy = iBlock - size;
      assert( iBuddy>=0 );
    }else{
      iBuddy = iBlock + size;
      if( iBuddy>=pMem->nBlock ) break;
    }
    if( pMem->aCtrl[iBuddy]!=(CTRL_FREE | iLogsize) ) break;
    memsys5Unlink(pMem, iBuddy, iLogsize);
    iLogsize++;
    if( iBuddy<iBlock ){
      pMem->aCtrl[iBuddy] = CTRL_FREE | iLogsize;
      pMem->aCtrl[iBlock] = 0;
      iBlock = iBuddy;
    }else{
      pMem->aCtrl[iBlock] = CTRL_FREE | iLogsize;
      pMem->aCtrl[iBuddy] = 0;
    }
    size *= 2;
  }

  pJunqi->free_cnt++;

  //log_a("free %d",pJunqi->free_cnt);
  memsys5Link(pMem, iBlock, iLogsize);
}

/*
** Round up a request size to the next valid allocation size.  If
** the allocation is too large to be handled by this allocation system,
** return 0.
**
** All allocations must be a power of two and must be expressed by a
** 32-bit signed integer.  Hence the largest allocation is 0x40000000
** or 1073741824 bytes.
*/
int memsys5Roundup(Mem5Global *pMem, int n){
  int iFullSz;
  if( n > 0x40000000 ) return 0;
  for(iFullSz=pMem->szAtom; iFullSz<n; iFullSz *= 2);
  return iFullSz;
}

/*
** Return the ceiling of the logarithm base 2 of iValue.
**
** Examples:   memsys5Log(1) -> 0
**             memsys5Log(2) -> 1
**             memsys5Log(4) -> 2
**             memsys5Log(5) -> 3
**             memsys5Log(8) -> 3
**             memsys5Log(9) -> 4
*/
static int memsys5Log(int iValue){
  int iLog;
  for(iLog=0; (iLog<(int)((sizeof(int)*8)-1)) && (1<<iLog)<iValue; iLog++);
  return iLog;
}

/*
** Initialize the memory allocator.
**
** This routine is not threadsafe.  The caller must be holding a mutex
** to prevent multiple threads from entering at the same time.
*/
int memsys5Init(Junqi *pJunqi, int nHeap, int mnReq){
  int ii;            /* Loop counter */
  int nByte;         /* Number of bytes of memory available to this allocator */
  u8 *zByte;         /* Memory usable by this allocator */
  int nMinLog;       /* Log base 2 of minimum allocation size in bytes */
  int iOffset;       /* An offset into pMem->aCtrl[] */


  /* The size of a Mem5Link object must be a power of two.  Verify that
  ** this is case.
  */
  assert( (sizeof(Mem5Link)&(sizeof(Mem5Link)-1))==0 );
  Mem5Global *pMem;

  pMem = (Mem5Global*)malloc(sizeof(Mem5Global)+nHeap);
  memset(pMem,0,sizeof(Mem5Global));
  pJunqi->pThreadMem = pMem;

  nByte = nHeap;
  zByte = (u8*)&pMem[1];
  assert( zByte!=0 );  /* sqlite3_config() does not allow otherwise */

  /* boundaries on sqlite3GlobalConfig.mnReq are enforced in sqlite3_config() */
  nMinLog = memsys5Log(mnReq);
  pMem->szAtom = (1<<nMinLog);
  while( (int)sizeof(Mem5Link)>pMem->szAtom ){
    pMem->szAtom = pMem->szAtom << 1;
  }

  pMem->nBlock = (nByte / (pMem->szAtom+sizeof(u8)));
  pMem->zPool = zByte;
  pMem->aCtrl = (u8 *)&pMem->zPool[pMem->nBlock*pMem->szAtom];

  for(ii=0; ii<=LOGMAX; ii++){
    pMem->aiFreelist[ii] = -1;
  }

  iOffset = 0;
  for(ii=LOGMAX; ii>=0; ii--){
    int nAlloc = (1<<ii);
    if( (iOffset+nAlloc)<=pMem->nBlock ){
      pMem->aCtrl[iOffset] = ii | CTRL_FREE;
      memsys5Link(pMem, iOffset, ii);
      iOffset += nAlloc;
    }
    assert((iOffset+nAlloc)>pMem->nBlock);
  }


  return 0;
}




