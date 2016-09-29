#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "MulticastErr.h"
#include "MulticastMsg.h"
#include "FecUnpad.h"
#include "Packet.h"
#include "fec.h"
#include "debug.h"

static char headerMark[5] = "bszh";

static char *padBuf = NULL;
static char *recoveryBuf = NULL;
static char *unpadBuf = NULL;
static int fecRecov[MAX_RECOVERY_PAGE_CNT];

static unsigned short pktsSerialNo;
static int startFlag = 0;

static int initUnpadOk = 0;

static int blkOffset;
static int maxPageNo = 0;		// 1个block中接收到的最大的page serial no
static int pageCntOfRecvBlk;	// 1个block中接收到的page总数
static int fecK;
static int fecN;
static int fecPageLen;


static PageInfo lPageInfo[MAX_PAGE_CNT_OF_BLK];
static PacketsInfo pktsInfo;

// 预设7种fec
const static int eccK[FEC_KN_NUM] = {1, 2, 3, 4, 5, 6, 7};
const static int eccC[FEC_KN_NUM] = {1, 1, 2, 2, 3, 3, 4};
const static int eccN[FEC_KN_NUM] = {2, 3, 5, 6, 8, 9, 11};

static fec_t *code[FEC_KN_NUM] = { NULL };
static unsigned int fecBlks[FEC_KN_NUM];
static int fecCodeNo;
static gf **lpInRecov;
static gf **lpOutRecov;
static int fecIndex[FEC_KN_NUM];

// for Debug
static int fTest = -1;
static int fTestOffset;
static char *fTestName = "test.mp4";


int FecUnpadInit()
{
	int i, r;
	PacketInfo *p;

	if(initUnpadOk == 1)
		return SUCCESS;


	unpadBuf = (char *)malloc(UNPAD_BUF_LEN + PAD_BUF_LEN + RECOVERY_BUF_LEN); 
	if(unpadBuf == NULL) {
		r = ERR_ALLOC_MEM;
		goto init_0;
	}
	padBuf = unpadBuf + UNPAD_BUF_LEN;
	recoveryBuf = unpadBuf + UNPAD_BUF_LEN + PAD_BUF_LEN;

	memset(unpadBuf, 0, UNPAD_BUF_LEN + PAD_BUF_LEN + RECOVERY_BUF_LEN);
	memset(lPageInfo, 0, sizeof(lPageInfo[MAX_PAGE_CNT_OF_BLK]));


	p = malloc(MAX_PACKET_CNT * sizeof(PacketInfo));
	if(p == NULL) {
		r = ERR_ALLOC_MEM;
		goto init_1;
	}

	pktsInfo.lInfo = p;

	for (i = 0; i < FEC_KN_NUM; i++) {
		code[i] = fec_new(eccK[i], eccN[i]);
		if (code[i] == NULL) {
			r = ERR_INIT_FEC;
			goto init_2;
		}
	}

	lpInRecov = (gf **)malloc(eccN[FEC_KN_NUM - 1] * sizeof(uchar *));
	if(lpInRecov == NULL) {
		r = ERR_INIT_FEC;
		goto init_2;
	}

	lpOutRecov = lpInRecov + eccK[FEC_KN_NUM-1];

	maxPageNo = 0;
	initUnpadOk = 1;

	return SUCCESS;

init_2:
	for (i = 0; i < FEC_KN_NUM; i++) {
		free(code[i]);
		code[i] = NULL;
	}

	free(p);
	pktsInfo.lInfo = NULL;
init_1:
	free(unpadBuf);
	unpadBuf = NULL;
init_0:
	return r;
}

int FecSetOutFile()
{
	if(initUnpadOk == 0)
		return ERR_NOT_INITOK;

	if(fTest == -1) {
		fTest = open(fTestName, O_RDWR | O_TRUNC | O_CREAT, 0666);
		if(fTest == -1) {
			return ERR_OPEN_FILE;
		}
		fTestOffset = 0;
	}

	return SUCCESS;
}

int FecUnpad()
{
	int r = SUCCESS;

	int i, j, k;
	int padOfs;
	char *p;
	char *recvPage;
	unsigned short blkSerialNo;
	PageInfo pageInfo;
	RecvInfo recvInfo;

	if(initUnpadOk == 0)
		return ERR_NOT_INITOK;

	// 接收队列消息
	r = multiCastRecvMsg(&recvInfo);
	if(r != SUCCESS) {
		return r;
	}

   	recvPage = recvInfo.ptr;
	if(recvPage == NULL) {
		return ERR_RECV_MSG;
	}

	// 判断header标志是否正确
	if(*(int *)recvPage != *(int *)headerMark)
		return ERR_HEAD_MARK;

	// 获取header
	PageHeader header;
	memcpy(&header, recvPage, sizeof(PageHeader));

	padOfs = header.pageSerialNo * header.fecPageLen;

	if(startFlag == 0) {
		pktsInfo.totalSize = header.pktsLen;
		pktsInfo.pktCnt = header.pktCnt;
		memcpy(pktsInfo.lInfo, recvPage+sizeof(PageHeader), header.pktCnt*sizeof(PacketInfo));

		lPageInfo[header.pageSerialNo].h = header;
		lPageInfo[header.pageSerialNo].data = padBuf + padOfs;
		lPageInfo[header.pageSerialNo].valid = 1;
		if(header.pageSerialNo > maxPageNo)
			maxPageNo = header.pageSerialNo;

		pktsSerialNo = header.pktsSerialNo;
		blkSerialNo = header.blkSerialNo;
		blkOffset = header.pktsOffset;
		fecK = header.fecK;
		fecN = header.fecN;
		fecPageLen = header.fecPageLen;
		pageCntOfRecvBlk = 1;
#if DEBUG
		printf("pktsSerialNo: %d blkSerialNo: %d blkOffset: %d fecK: %d fecN: %d fecPageLen: %d\n", pktsSerialNo, blkSerialNo, blkOffset, fecK, fecN, fecPageLen);
		printf("pageIndex: %d pageSerialNo: %d padOfs: %d\n",recvInfo.index, header.pageSerialNo, padOfs);
#endif
		memcpy(padBuf+padOfs, recvPage+sizeof(PageHeader)+header.pktCnt*sizeof(PacketInfo), header.fecPageLen);
		startFlag = 1;
		return SUCCESS;
	}

	// 判断是否不同的block
	if(header.blkSerialNo != blkSerialNo) {

		// 新收到的page属于另外一个block，处理上一个block
		// 判断是否满足FEC纠错
		if(pageCntOfRecvBlk < fecK) {
#if DEBUG
			printf("lost too many page, recv: %d send: %d\n\n", pageCntOfRecvBlk, fecN);
			printf("maxPageNo: %d\n", maxPageNo);
#endif
			// 收到的page太少，无法恢复，只能将收到的数据部分复制
			for(i=0; i<=maxPageNo; i++) {
				pageInfo = lPageInfo[i];
				if(pageInfo.valid == 0)
					continue;

				if(pageInfo.h.pageSerialNo < pageInfo.h.fecK) {
					// 是数据
					memcpy(unpadBuf+blkOffset+pageInfo.h.pageSerialNo * pageInfo.h.fecPageLen,
							pageInfo.data,
							pageInfo.h.fecPageLen
						);
				}
			}
			r = ERR_ENOUGH_PAGE;
		} else {
			// page数足够纠错
			int needFec = 0;

			j = fecK;
			k = 0;
			for(i=0; i<fecK; i++) {
				pageInfo = lPageInfo[i];
				if(pageInfo.valid == 0) {
					for( ; j<fecN ; j++) {
						if(lPageInfo[j].valid == 0)
							continue;
						break;
					}
					fecIndex[i]   = j;
					lpInRecov[i]  = lPageInfo[j].data;
					fecRecov[k]   = i;
					lpOutRecov[k] = recoveryBuf + k * fecPageLen;
					++j;
					++k;
					needFec = 1;
				} else {
					fecIndex[i] = i;
					lpInRecov[i] = lPageInfo[i].data;
				}
			}

			// FEC纠错
			if(needFec) {
#if DEBUG
				printf("need fec\n");
#endif
				fec_decode(code[fecK-1], (const gf **)lpInRecov, lpOutRecov, fecIndex, fecPageLen);
				for(i=0; i<k; i++) {
					memcpy(padBuf+(fecRecov[i]*fecPageLen), lpOutRecov[i], fecPageLen);
				}
			}

			// 复制恢复数据到unpadBuf
#if DEBUG
			printf("copy block data blkOffset: %d len: %d\n\n", blkOffset, fecK*fecPageLen);
#endif
			memcpy(unpadBuf+blkOffset, padBuf, fecK*fecPageLen);

			r = SUCCESS;
		}


		maxPageNo = 0;
		memset(padBuf, 0, PAD_BUF_LEN + RECOVERY_BUF_LEN);
		for(i=0; i<MAX_PAGE_CNT_OF_BLK; i++) {
			lPageInfo[i].valid = 0;
		}
		pageCntOfRecvBlk = 0;

		blkSerialNo = header.blkSerialNo;
		blkOffset = header.pktsOffset;
		fecK = header.fecK;
		fecN = header.fecN;
		fecPageLen = header.fecPageLen;
	}

	// 判断是否不同的video packets
	if(header.pktsSerialNo != pktsSerialNo) {
		// 处理video packets

		// 保存到文件中，测试正确性
		if(fTest != -1) {
#if DEBUG
			printf("Write file: offset: %d size: %d\n", fTestOffset, pktsInfo.totalSize);
#endif
			write(fTest, unpadBuf, pktsInfo.totalSize);
			fTestOffset += pktsInfo.totalSize;
		}

		// 生成视频packet，解压到显示缓冲区
		for(i=0 ; i<pktsInfo.pktCnt; i++) {
			// unpadBuf+lInfo[i].offset lInfo[i].size
		}
#if DEBUG
		printf("New pkts: pktsSerialNo: %d\n", header.pktsSerialNo);
#endif

		// 初始化buf
		memset(unpadBuf, 0, UNPAD_BUF_LEN);

		// 新的packets
		pktsInfo.totalSize = header.pktsLen;
		pktsInfo.pktCnt = header.pktCnt;
		memcpy(pktsInfo.lInfo, recvPage+sizeof(PageHeader), header.pktCnt*sizeof(PacketInfo));

		pktsSerialNo = header.pktsSerialNo;
	}

#if DEBUG
	if(pageCntOfRecvBlk == 0) {
		printf("new blkSerialNo: %d\n", header.blkSerialNo);
		printf("pktsSerialNo: %d blkSerialNo: %d blkOffset: %d fecK: %d fecN: %d fecPageLen: %d\n", pktsSerialNo, blkSerialNo, blkOffset, fecK, fecN, fecPageLen);
	}
	printf("pageIndex: %d pageSerialNo: %d padOfs: %d\n",recvInfo.index, header.pageSerialNo, padOfs);
#endif

	lPageInfo[header.pageSerialNo].h = header;
	lPageInfo[header.pageSerialNo].data = padBuf + padOfs;
	lPageInfo[header.pageSerialNo].valid = 1;
	if(header.pageSerialNo > maxPageNo)
		maxPageNo = header.pageSerialNo;

	pageCntOfRecvBlk += 1;
	memcpy(padBuf+padOfs, recvPage+sizeof(PageHeader)+header.pktCnt*sizeof(PacketInfo), header.fecPageLen);

	return r;
}

void FecUnpadClose()
{
	int i;

	if(initUnpadOk == 0)
		return;

	if(fTest != -1) {
		close(fTest);
		fTest = -1;
		fTestOffset = 0;
	}

	for (i = 0; i < FEC_KN_NUM; i++) {
		free(code[i]);
		code[i] = NULL;
	}

	free(pktsInfo.lInfo);
	pktsInfo.lInfo = NULL;

	free(unpadBuf);
	unpadBuf = NULL;

	free(lpInRecov);
	lpInRecov = NULL;

	startFlag = 0;
	initUnpadOk = 0;
}

