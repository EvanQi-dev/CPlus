#include <stdio.h>
#include "..\include\HLog.h"
#include "MultiCast.h"
#include "FecPad.h"
extern "C" {
#include "msock.h"
}

MultiCast::MultiCast() : QThread()
{
	initOk = true;
	queueSem = new QSemaphore(0);
	if (queueSem == nullptr) {
		LOG_ERROR("MultiCast new queueSem error");
	}
	queueMutex = new QMutex();
	if (queueMutex == nullptr) {
		LOG_ERROR("MultiCast new queueMutex error");
	}
	fecPad = new FecPad();
	if (fecPad == nullptr) {
		LOG_ERROR("MultiCast new fecPad error");
	}

	if ((queueSem == nullptr) || (queueMutex == nullptr) || (fecPad == nullptr))
		initOk = false;

	mcastIp = DEF_MULTICAST_IP;
	mcastPort = DEF_MULTICAST_PORT;

	sendSock = mcast_send_socket(mcastIp.toUtf8().data(), mcastPort.toUtf8().data(), mcastTTL, &mcastAddr);
	if (sendSock == -1) {
		LOG_ERROR("MultiCast get send socket error");
		initOk = false;
	}
}

MultiCast::~MultiCast()
{
	if (queueSem != nullptr) delete queueSem;
	if (queueMutex != nullptr) delete queueMutex;
}

void MultiCast::startMultiCast()
{
	if (initOk == false)
		return;

	if (runFlag == false) {
		runFlag = true;
		start();
	}
}

void MultiCast::stopMultiCast()
{
	if (initOk == false)
		return;

	if (runFlag) {
		runFlag = false;
	}
}

// 复制视频数据并放入队列
int MultiCast::enqueueBlock(uchar *data, int size)
{
	if (initOk == false)
		return -1;

	uchar *dst = new uchar[size];
	if (dst == nullptr) {
		LOG_ERROR("Ask memory for video packet error");
		return -2;
	}
	memcpy(dst, data, size);
	Packet b = { dst, size };

	queueMutex->lock();
	pktQueue.enqueue(b);
	queueMutex->unlock();

	queueSem->release(1);

	return 0;
}

int pageInx = 0;

void MultiCast::run(void)
{
	FILE *f = fopen("test1.mpg", "wb");
	Packet b;
	while (runFlag) {
		queueSem->acquire(1);
		queueMutex->lock();
		b = pktQueue.dequeue();
		queueMutex->unlock();

		// 处理发送数据
		char *data;
		int cnt;
		int size;
		int r;
		r = fecPad->GetPacket(b, (unsigned char **)&data, &cnt, &size);
		if (r == 0 && cnt > 0) {
			for (int i = 0; i < cnt; i++) {
				PageHeader header;
				memcpy(&header, data + i*size, sizeof(PageHeader));
				if (header.pageSerialNo < header.fecK)
					fwrite(data + i*size, 1, size, f);

				if (pageInx++ % 2 == 1)
					continue;

				if (sendto(sendSock, data + i*size, size, 0, mcastAddr->ai_addr, mcastAddr->ai_addrlen) != size) {
					LOG_ERROR(QString("Send data error, page cnt: %1 i: %2").arg(cnt).arg(i));
				}
			}
			delete data;
		}
	}

	// 释放资源
	int noProcCnt = queueSem->available();
	queueSem->acquire(noProcCnt);

	int size = pktQueue.size();
	for (int i = 0; i < size; i++) {
		b = pktQueue.dequeue();
		if (b.data != nullptr)
			delete b.data;
	}
}

