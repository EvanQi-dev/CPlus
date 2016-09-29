
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "MulticastRecv.h"
#include "MulticastErr.h"
#include "MulticastMsg.h"
#include "FecUnpad.h"

void *recvThread(void *);
void *unpadTrhead(void *);

int main(int argc, char* argv[])
{
	int r;
	pthread_t threadRecv, threadUnpad;

	r = multiCastRecvInit("224.9.2.20", "16113");
	if(r != SUCCESS) {
		printf("Init error, return code: %d\n", r);
		return r;
	}

	r = multiCastCreateMsg();
	if(r != SUCCESS) {
		printf("Init message error, return code: %d\n", r);
		return r;
	}

	r = FecUnpadInit();
	if(r != SUCCESS) {
		printf("Init fecUnpad error, return code: %d\n", r);
		return r;
	}

	r = FecSetOutFile();
	if(r != SUCCESS) {
		printf("FecUnpad open file error, return code: %d\n", r);
		return r;
	}

	r = pthread_create(&threadRecv, NULL, recvThread, NULL);
	if(r != 0) {
		printf("Create threadRecv error, return code: %d\n", r);
		return r;
	}
	
	r = pthread_create(&threadUnpad, NULL, unpadTrhead, NULL);
	if(r != 0) {
		printf("Create threadRecv error, return code: %d\n", r);
		return r;
	}

	while(1) {
		sleep(100);
	}

	return 0;
}


void * recvThread(void *arg)
{
	int r;

	while(1) {
		r = multiCastRecvPage();
		if(r != SUCCESS) {
			printf("recv error, return code: %d\n", r);
		}
	}
}

void * unpadTrhead(void *arg)
{
	int r;

	while(1) {
		r = FecUnpad();
		if(r != SUCCESS) {
			printf("unpad error, return code: %d\n", r);
		}
	}
}



