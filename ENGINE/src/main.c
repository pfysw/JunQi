/*
 * main.c
 *
 *  Created on: Aug 17, 2018
 *      Author: Administrator
 */

#include "junqi.h"
#include "comm.h"
#include "engine.h"
#include <time.h>


int main(int argc, char *argv[])
{
	Junqi *pJunqi = JunqiOpen();
	pthread_t t1;

	setvbuf(stdout, NULL, _IONBF, 0);

	t1 = CreatCommThread(pJunqi);
	CreatEngineThread(pJunqi);

	pthread_join(t1,NULL);

	return 0;
}

