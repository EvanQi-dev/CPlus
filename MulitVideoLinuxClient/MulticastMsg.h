#pragma once

typedef struct RecvInfo {
	char *ptr;
	int index;
} RecvInfo;

typedef struct MsgBuf {
	long mtype;  /* message type, must be > 0 */
	RecvInfo recvInfo;
} MsgBuf;

int multiCastCreateMsg();
int multiCastSendMsg(RecvInfo info);
int multiCastRecvMsg(RecvInfo *info);
void multiCastCloseMsg();
