/*
 * evaluate.h
 *
 *  Created on: Sep 13, 2018
 *      Author: Administrator
 */

#ifndef EVALUATE_H_
#define EVALUATE_H_
#include "type.h"

typedef struct Value_Parameter_t
{
	int vAllChess;
	u8  vChess[14];
	u8  vDarkLand;
	u8  vDarkBomb;
	u8  vDarkJunqi;
}Value_Parameter;

void InitValuePara(Value_Parameter *p);
int EvalSituation(Junqi *pJunqi);

#endif /* ALPHA_BETA_H_ */
