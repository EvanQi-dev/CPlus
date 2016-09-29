#include "../include/HLog.h"
#include "FecPad.h"
extern "C" {
#include "fec.h"
};

// Ԥ��7��fec
const int eccK[FEC_KN_NUM] = {1, 2, 3, 4, 5, 6, 7};
const int eccC[FEC_KN_NUM] = {1, 1, 2, 2, 3, 3, 4};
const int eccN[FEC_KN_NUM] = {2, 3, 5, 6, 8, 9, 11};

FecPad::FecPad()
{
	initOk = true;

	memcpy(header.mark, "bszh", 4);
	header.version = 0x0001;
	headerSize = sizeof(PageHeader);

	for (int i = 0; i < FEC_KN_NUM; i++) {
		code[i] = fec_new(eccK[i], eccN[i]);
		if (code[i] == nullptr) {
			initOk = false;
			break;
		}
	}

	lSrc = new unsigned char *[eccK[FEC_KN_NUM - 1]];
	if (lSrc == nullptr)
		initOk = false;

	lFec = new unsigned char *[eccC[FEC_KN_NUM - 1]];
	if (lFec == nullptr)
		initOk = false;

	fecBuf = new unsigned char[eccC[FEC_KN_NUM - 1]*PAGE_SIZE];
	if (fecBuf == nullptr)
		initOk = false;

	if (initOk == false) {
		LOG_ERROR("FecPad ask memory error");
	}
}

FecPad::~FecPad()
{
	if (initOk) {
		for (int i = 0; i < FEC_KN_NUM; i++) {
			if (code[i] != nullptr) {
				fec_free(code[i]);
				code[i] = nullptr;
			}
		}

		if (lSrc != nullptr)
			delete lSrc;

		if (lFec != nullptr)
			delete lFec;
	}
}

// ����Ƿ���Է��
bool FecPad::chkPacketsLen()
{
	pktCnt = lPacket.size();
	pktsLen = 0;
	for (int i = 0; i < pktCnt; i++) {
		pktsLen += lPacket[i].size;
	}

	fecBytes = PAGE_SIZE-sizeof(PageHeader)-pktCnt*8;

	pageCnt = (pktsLen + fecBytes - 1) / fecBytes;
	if (pageCnt >= 4)
		return true;
	else
		return false;
}

// ����lPacket�б��ж��Ƿ��������ͳ�ȥ
// input:
//     pkt    video packet
//     s_dat  ���������ݵ�ַ
//     s_cnt  �����
//     s_size �����page��С
// return:
//     0     OK
//     other error
int FecPad::GetPacket(Packet pkt, unsigned char **s_dat, int *s_cnt, int *s_size)
{
	*s_cnt = 0;
	if (initOk == false) {
		return -1;
	}

	lPacket.append(pkt);

	bool bCanPad = chkPacketsLen();
	if (bCanPad == false) {
		return 0;
	}

	int r = 0;
	int padPageCnt = 0;
	int infoSize = sizeof(PacketInfo);
	int infoBufSize = pktCnt * infoSize;

	int i;
	// ���pageCnt < 8��ֱ�ӷ��
	if (pageCnt < 8) {
		fecCodeNo = pageCnt - 1;

		padData = new unsigned char[eccN[fecCodeNo]*PAGE_SIZE];
		if (padData == nullptr) {
			LOG_ERROR("Ask padData memory error");
			r = -2;
			goto err_0;
		}

		int l = eccN[fecCodeNo] * fecBytes + infoBufSize;
		tmpData = new unsigned char[l];
		if (tmpData == nullptr) {
			LOG_ERROR("Ask tmpData memory error");
			r = -3;
			goto err_1;
		}
		memset(tmpData, 0, l);
		infoBuf = tmpData + eccN[fecCodeNo] * fecBytes;

		// ����FEC
		int offset = 0;
		int pktSize;
		PacketInfo info;
		unsigned char *p = tmpData;
		lPacketInfo.clear();
		for (i = 0; i < lPacket.size(); i++) {
			pktSize = lPacket[i].size;

			info.offset = offset;
			info.size = pktSize;
			lPacketInfo.append(info);

			memcpy(p, lPacket[i].data, pktSize);
			delete lPacket[i].data;
			p += pktSize;
			offset += pktSize;
		}

		p = tmpData;
		for (i = 0; i < eccK[fecCodeNo]; i++) {
			lSrc[i] = p;
			p += fecBytes;
		}
		for (i = 0; i < eccC[fecCodeNo]; i++) {
			fecBlks[i] = eccK[fecCodeNo] + i;
			lFec[i] = p;
			p += fecBytes;
		}
		fec_encode(code[fecCodeNo], lSrc, lFec, fecBlks, eccC[fecCodeNo], fecBytes);

		// ���
		header.pktsSerialNo = pktsSerialNo++;
		header.pktsLen = pktsLen;
		header.pktsOffset = 0;
		header.blkSerialNo = blkSerialNo++;
		header.fecK = eccK[fecCodeNo];
		header.fecN = eccN[fecCodeNo];
		header.fecPageLen = fecBytes;
		header.pktCnt = pktCnt;

		// ��packet��ƫ�ƺͳ����ȷŵ�infoBuf�У����ʱһ�𿽱�
		p = infoBuf;
		for (i = 0; i < lPacketInfo.size(); i++) {
			memcpy(p, &lPacketInfo[i], infoSize);
			p += infoSize;
		}

		p = tmpData;
		unsigned char *d = padData;
		for (i = 0; i < eccN[fecCodeNo]; i++) {
			header.pageSerialNo = i;
			memcpy(d, &header, headerSize);		// header
			d += headerSize;

			memcpy(d, infoBuf, infoBufSize);	// info
			d += infoBufSize;

			memcpy(d, p, fecBytes);	// data
			d += fecBytes;
			p += fecBytes;
		}

		padPageCnt = eccN[fecCodeNo];
	}
	else {
		// ���pageCnt >= 8, ÿ4 pagesѭ�����
		// ʣ�಻��4 pages�������
		fecCodeNo = FEC_DEF_NO;

		int cnt = pageCnt + (pageCnt + 2) / 2;	// ����һ������
		padData = new unsigned char[cnt*PAGE_SIZE];
		if (padData == nullptr) {
			LOG_ERROR("Ask padData memory error");
			r = -2;
			goto err_0;
		}

		tmpData = new unsigned char[cnt*fecBytes + infoBufSize];
		if (tmpData == nullptr) {
			LOG_ERROR("Ask tmpData memory error");
			r = -3;
			goto err_1;
		}
		infoBuf = tmpData + cnt*fecBytes;

		// video packetƫ�ƺͳ���д��infoBuf
		int offset = 0;
		int pktSize;
		PacketInfo info;
		unsigned char *p = tmpData;
		lPacketInfo.clear();
		for (i = 0; i < lPacket.size(); i++) {
			pktSize = lPacket[i].size;

			info.offset = offset;
			info.size   = pktSize;
			lPacketInfo.append(info);

			memcpy(p, lPacket[i].data, pktSize);
			delete lPacket[i].data;
			p      += pktSize;
			offset += pktSize;
		}

		header.pktsSerialNo = pktsSerialNo++;
		header.pktsLen = pktsLen;
		header.pktsOffset = 0;
		header.fecK = eccK[fecCodeNo];
		header.fecN = eccN[fecCodeNo];
		header.fecPageLen = fecBytes;
		header.pktCnt = pktCnt;

		// ��packet��ƫ�ƺͳ����ȷŵ�infoBuf�У����ʱһ�𿽱�
		p = infoBuf;
		for (i = 0; i < lPacketInfo.size(); i++) {
			memcpy(p, &lPacketInfo[i], infoSize);
			p += infoSize;
		}

		// ����FEC
		int tCnt = pageCnt;
		p = tmpData;
		unsigned char *s;
		unsigned char *f;
		unsigned char *d = padData;
		do {
			s = p;
			for (i = 0; i < eccK[fecCodeNo]; i++) {
				lSrc[i] = p;
				p += fecBytes;
			}

			f = fecBuf;
			for (i = 0; i < eccC[fecCodeNo]; i++) {
				fecBlks[i] = eccK[fecCodeNo] + i;
				lFec[i] = f;
				f += fecBytes;
			}
			fec_encode(code[fecCodeNo], lSrc, lFec, fecBlks, eccC[fecCodeNo], fecBytes);

			// ���
			header.blkSerialNo = blkSerialNo;
			for (i = 0; i < eccK[fecCodeNo]; i++) {
				header.pageSerialNo = i;
				memcpy(d, &header, headerSize);		// header
				d += headerSize;

				memcpy(d, infoBuf, infoBufSize);	// info
				d += infoBufSize;

				memcpy(d, s, fecBytes);	// data
				d += fecBytes;
				s += fecBytes;
			}

			f = fecBuf;
			for (int j= 0; j < eccC[fecCodeNo]; i++, j++) {
				header.pageSerialNo = i;
				memcpy(d, &header, headerSize);		// header
				d += headerSize;

				memcpy(d, infoBuf, infoBufSize);	// info
				d += infoBufSize;

				memcpy(d, f, fecBytes);	// data
				d += fecBytes;
				f += fecBytes;
			}

			header.pktsOffset += fecBytes * eccK[fecCodeNo];
			++blkSerialNo;

			padPageCnt += eccN[fecCodeNo];

			tCnt -= eccK[fecCodeNo];
		} while (tCnt >= eccK[fecCodeNo]);

		// ʣ�಻��4 pages, ����FEC�ͷ��
		if (tCnt > 0) {
			fecCodeNo = tCnt - 1;
			s = p;

			// FEC
			for (i = 0; i < eccK[fecCodeNo]; i++) {
				lSrc[i] = p;
				p += fecBytes;
			}

			f = fecBuf;
			for (i = 0; i < eccC[fecCodeNo]; i++) {
				fecBlks[i] = eccK[fecCodeNo] + i;
				lFec[i] = f;
				f += fecBytes;
			}
			fec_encode(code[fecCodeNo], lSrc, lFec, fecBlks, eccC[fecCodeNo], fecBytes);

			// ���
			header.blkSerialNo = blkSerialNo++;
			header.fecK = eccK[fecCodeNo];
			header.fecN = eccN[fecCodeNo];
			for (i = 0; i < eccK[fecCodeNo]; i++) {
				header.pageSerialNo = i;
				memcpy(d, &header, headerSize);		// header
				d += headerSize;

				memcpy(d, infoBuf, infoBufSize);	// info
				d += infoBufSize;

				memcpy(d, s, fecBytes);	// data
				d += fecBytes;
				s += fecBytes;
			}

			f = fecBuf;
			for (int j= 0; j < eccC[fecCodeNo]; i++, j++) {
				header.pageSerialNo = i;
				memcpy(d, &header, headerSize);		// header
				d += headerSize;

				memcpy(d, infoBuf, infoBufSize);	// info
				d += infoBufSize;

				memcpy(d, f, fecBytes);	// data
				d += fecBytes;
				f += fecBytes;
			}

			padPageCnt += eccN[fecCodeNo];
		}
	}

	*s_dat = padData;
	*s_cnt = padPageCnt;
	*s_size = PAGE_SIZE;

	delete tmpData;
err_1:
	if (r != 0) delete padData;
err_0:
	lPacket.clear();
	return r;
}

int FecPad::padPacketData()
{
	return 0;
}
