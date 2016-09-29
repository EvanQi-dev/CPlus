#pragma once

#include <QThread>
#include <QSemaphore>
#include <QQueue>
#include "Packet.h"
#include "Ws2tcpip.h"

#define DEF_MULTICAST_IP   "224.9.2.20"
#define DEF_MULTICAST_PORT "16113"

class FecPad;

class MultiCast : public QThread
{
	Q_OBJECT

public:
	MultiCast();
	~MultiCast();

	int enqueueBlock(uchar *data, int size);
	void startMultiCast();
	void stopMultiCast();

protected:
	virtual void run(void);

private:
	bool initOk = false;

	QSemaphore *queueSem = nullptr;
	QMutex *queueMutex = nullptr;
	QQueue<Packet> pktQueue;

	SOCKET sendSock = -1;
	struct addrinfo *mcastAddr;
	QString mcastIp;
	QString mcastPort;
	int mcastTTL = 1;

	int runFlag = false;

	FecPad *fecPad = nullptr;
};
