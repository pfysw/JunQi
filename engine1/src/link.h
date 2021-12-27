/*
 * link.h
 *
 *  Created on: Dec 27, 2021
 *      Author: Administrator
 */

#ifndef LINK_H_
#define LINK_H_
#include "jtype.h"

typedef struct LinkNode LinkNode;
struct LinkNode
{
    LinkNode *pNext;
    LinkNode *pPre;
    void *pVal;
    u8 isHead;
    u8 id;
};

void* Malloc2(Junqi* pJunqi,u32 size);
LinkNode *NewLinkNode2(Junqi* pJunqi,void *pVal,int size);
LinkNode *NewLinkHead2(Junqi* pJunqi,void *pVal,int size);
void InsertLinkNode(Junqi* pJunqi,LinkNode *pPre,LinkNode *p);

#endif /* LINK_H_ */
