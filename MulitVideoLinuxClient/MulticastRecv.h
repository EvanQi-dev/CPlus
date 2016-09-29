#pragma once

#define RECV_PAGE_CNT 3276
#define PAGE_SIZE     1280
#define MULTICAST_SOCKET_RECVBUF	(3*1024*1024)

typedef struct Page {
	int  isUse;
	int  size;
	char *data;
} Page;

int multiCastRecvInit(char *ip, char *port);
int multiCastRecvPage();
void multiCastRecvClose();
