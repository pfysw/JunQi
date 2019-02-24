/*
 * evaluate.h
 *
 *  Created on: Sep 13, 2018
 *      Author: Administrator
 */

#ifndef EVALUATE_H_
#define EVALUATE_H_
#include "type.h"

extern const u8 gChessValue[14];
typedef struct Value_Parameter_t
{
	int vAllChess;
	u8  vChess[14];
	u8  vDarkLand;
	u8  vDarkBomb;
	u8  vDarkJunqi;
	u8  vPathLand;
	u8  vPathChess;
	u8  vDanger;

}Value_Parameter;

void InitValuePara(Value_Parameter *p);
int EvalSituation(Junqi *pJunqi, u8 isInit);
int GetConnectValue(Junqi *pJunqi, int iDir);
void ReSetBombValue(Junqi *pJunqi);
void SetMaxType(Junqi *pJunqi);

#endif /* ALPHA_BETA_H_ */
