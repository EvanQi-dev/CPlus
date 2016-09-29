
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msock.h"
#include "Packet.h"
#include "MulticastErr.h"
#include "MulticastRecv.h"
#include "MulticastMsg.h"
#include "debug.h"


static int initRecvOk = 0;

static Page recvPage[RECV_PAGE_CNT];
static int pageInx;

static SOCKET recvSock = -1;
static char *recvBuf = NULL;

// 初始化接收组播数据所用内存，缓冲。获取接收socket
int multiCastRecvInit(char *ip, char *port)
{
	int  i;
	char *data;
	char *p;
	int r = SUCCESS;

	if(initRecvOk == 1)
		return r;


	pageInx = 0;

	if(*ip == 0 || *port==0) {
		r = ERR_PARAM_VAL;
		goto mr_0;
	}

	recvBuf = malloc(2*PAGE_SIZE * sizeof(char));
	if(recvBuf == NULL) {
		r = ERR_ALLOC_MEM;
		goto mr_0;
	}

	data = malloc(RECV_PAGE_CNT*PAGE_SIZE*sizeof(char));
	if(data == NULL) {
		r = ERR_ALLOC_MEM;
		goto mr_1;
	}

	p = data;
	for(i=0; i<RECV_PAGE_CNT; i++) {
		recvPage[i].data = p;
	    p += PAGE_SIZE;
	}

	recvSock = mcast_recv_socket(ip, port, MULTICAST_SOCKET_RECVBUF);
	if(recvSock < 0) {
		r = ERR_GET_SOCKET;
		goto mr_2;
	}

	initRecvOk = 1;
	return r;


mr_2:
	free(data);
mr_1:
	free(recvBuf);
	recvBuf = NULL;
mr_0:
	initRecvOk = 0;
	return r;
}


int multiCastRecvPage()
{
	int r;
	int bytes;
	RecvInfo info;

	if ((bytes = recvfrom(recvSock, recvBuf, PAGE_SIZE, 0, NULL, 0)) < 0) {
		return ERR_RECV_DATA;
	}

#if 0
	PageHeader header;
	memcpy(&header, recvBuf, sizeof(header));
	if(bytes != PAGE_SIZE) {
		printf("Error bytes: %d ------------------------\n", bytes);
	}
	printf("pktsSerialNo: %d pktsLen: %d pktsOffset: %d blkSerialNo: %d pageSerialNo: %d pktCnt: %d pageInx: %d\n",
			header.pktsSerialNo,
			header.pktsLen,
			header.pktsOffset,
			header.blkSerialNo,
			header.pageSerialNo,
			header.pktCnt,
			pageInx
		  );
#endif

	memcpy(recvPage[pageInx].data, recvBuf, bytes);
	recvPage[pageInx].isUse = 1;

	info.ptr = recvPage[pageInx].data;
	info.index = pageInx;

	r = multiCastSendMsg(info);
	if(r != SUCCESS) {
		return r;
	}

	pageInx++;
	if(pageInx == RECV_PAGE_CNT)
		pageInx = 0;

	return SUCCESS;
}


void multiCastRecvClose()
{
	if(initRecvOk == 1) {
		int  i;
		char *c;
		Page *p;

		close(recvSock);
		recvSock = -1;

		c = recvPage[0].data;
		if(c != NULL)
			free(c);

		free(recvBuf);
		recvBuf = NULL;

		initRecvOk = 0;
	}
}



