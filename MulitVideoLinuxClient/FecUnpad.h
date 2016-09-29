#pragma once

#include "Packet.h"

#define MAX_RECOVERY_PAGE_CNT 4
#define DEF_PAGE_CNT_OF_BLK	  6
#define MAX_PAGE_CNT_OF_BLK   12
#define MAX_PACKET_CNT        100

#define RECOVERY_BUF_LEN    (MAX_RECOVERY_PAGE_CNT*1280)
#define PAD_BUF_LEN         (MAX_PAGE_CNT_OF_BLK*1280)
#define UNPAD_BUF_LEN		(2*1024*1024)

#define FEC_KN_NUM	7

typedef unsigned char uchar;

typedef struct PageInfo {
	PageHeader h;
	char *data;
	int  valid;
} PageInfo;


int FecUnpadInit();
int FecUnpad();
void FecUnpadClose();
