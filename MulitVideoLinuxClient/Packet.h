#pragma once

#pragma pack(push, 1)

typedef struct PageHeader {
	unsigned char  mark[4];
 	unsigned short version;
	unsigned short pktsSerialNo;
	unsigned int   pktsLen;
	unsigned int   pktsOffset;	// block有效数据的偏移
	unsigned short blkSerialNo;
	unsigned short pageSerialNo;
	unsigned short fecK;
	unsigned short fecN;
	unsigned short fecPageLen;
	unsigned short pktCnt;
} PageHeader;

typedef struct PacketInfo {
	int offset;
	int size;
} PacketInfo;

#pragma pack(pop)

typedef struct Packet {
	unsigned char *data;
	int size;
} Packet;

typedef struct PacketsInfo {
	int totalSize;
	int pktCnt;
	PacketInfo *lInfo;
} PacketsInfo;
