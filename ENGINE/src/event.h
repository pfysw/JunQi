/*
 * event.h
 *
 *  Created on: Aug 27, 2018
 *      Author: Administrator
 */

#ifndef EVENT_H_
#define EVENT_H_
#include "engine.h"
#include "junqi.h"

#define SETBIT(V,I)      V[I>>3] |= (1<<(I&7))
#define CLEARBIT(V,I)    V[I>>3] &= ~(1<<(I&7))
#define TESTBIT(V,I)     (V[I>>3]&(1<<(I&7)))!=0

extern char *aTypeName[14];

u8 ComeInCamp(Engine *pEngine);
void CheckCampEvent(Engine *pEngine,BoardChess *pChess);
u8 ProBombEvent(Engine *pEngine);
void CheckBombEvent(Engine *pEngine);
u8 ProEatEvent(Engine *pEngine);
void CheckEatEvent(Engine *pEngine);
u8 ProJunqiEvent(Engine *pEngine);
void CheckJunqiEvent(Engine *pEngine);
void ChecAttackEvent(Engine *pEngine);
void PopDarkJunqiChess(Engine *pEngine);

void CheckGLobalInfo(Engine *pEngine);
#endif /* EVENT_H_ */
