#pragma once

#include <QList>
#include "Packet.h"

#define FEC_KN_NUM 7
#define FEC_DEF_NO 3
#define PAGE_SIZE  1280
#define MINI_BLK_SIZE 5120

struct fec_t;

class FecPad
{
public:
	FecPad();
	~FecPad();

	int GetPacket(Packet pkt, unsigned char **s_dat, int *s_cnt, int *s_size);	// 放入lPacket列表，判断是否封包，发送出去

private:
	bool chkPacketsLen();	// 检查是否已经可以封包
	int padPacketData();
	void addSerialNo(int *n);

	fec_t *code[FEC_KN_NUM];
	unsigned int fecBlks[FEC_KN_NUM];
	int fecCodeNo = FEC_DEF_NO;
	unsigned char **lSrc;
	unsigned char **lFec;

	unsigned char *padData;	// 存放加FEC封包后的数据
	unsigned char *tmpData;	// 辅助buffer
	unsigned char *infoBuf;
	unsigned char *fecBuf;

	QList<Packet> lPacket;
	QList<PacketInfo> lPacketInfo;

	unsigned short pktsSerialNo = 0;
	unsigned short blkSerialNo = 0;

	int pktsLen;
	int pktCnt;
	int pageCnt;
	int fecBytes;	// 每Page有效负载数据

	PageHeader header;
	int headerSize;

	bool initOk = false;
};
