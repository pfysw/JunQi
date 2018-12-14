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
#include <signal.h>

int main(int argc, char *argv[])
{
	Junqi *pJunqi = JunqiOpen();
	pthread_t t1;

	setvbuf(stdout, NULL, _IONBF, 0);

	t1 = CreatCommThread(pJunqi);
	CreatEngineThread(pJunqi);
	CreatPrintThread(pJunqi);

	//只能在主线程中创建线程才能多核
	//在其他线程中创建的新线程都与该线程使用同一个cpu
	//CreatSearchThread(pJunqi);
	//CreatSearchThread(pJunqi);

	pthread_join(t1,NULL);

	return 0;
}

