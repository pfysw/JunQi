/*
 * link.c
 *
 *  Created on: Dec 27, 2021
 *      Author: Administrator
 */
#include "junqi.h"
#include "link.h"

//全局申请不释放
void* Malloc2(Junqi* pJunqi,u32 size)
{
    void *p = malloc(size);
    memset(p,0,size);
    return p;
}


LinkNode *NewLinkNode2(Junqi* pJunqi,void *pVal,int size)
{
    LinkNode *p;
    p = (LinkNode *)Malloc2(pJunqi,size);
    memset(p,0,size);
    p->pVal = pVal;
    return p;
}

LinkNode *NewLinkHead2(Junqi* pJunqi,void *pVal,int size)
{
    LinkNode *p;
    p = NewLinkNode2(pJunqi,pVal,size);
    p->pNext = p;
    p->pPre = p;
    p->isHead = 1;
    return p;
}
