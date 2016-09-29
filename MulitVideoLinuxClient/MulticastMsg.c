#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "MulticastMsg.h"
#include "MulticastErr.h"


static int MulticastMsgId = -1;
static char *keyMark = "bszh";

int multiCastCreateMsg()
{
	key_t key;
	struct msqid_ds buf;

redo:
	memcpy(&key, keyMark, 4);
	MulticastMsgId = msgget(key, 0666 | IPC_CREAT);
	if(MulticastMsgId == -1) {
		return ERR_GET_MSGID;
	}

	msgctl(MulticastMsgId, IPC_STAT, &buf);
	if(buf.msg_qnum > 0) {
		printf("clear old message\n");
		msgctl(MulticastMsgId, IPC_RMID, 0);
		goto redo;
	}

	return SUCCESS;
}


int multiCastSendMsg(RecvInfo info)
{
	MsgBuf b;
	b.mtype = 1;
	b.recvInfo = info;

	if(msgsnd(MulticastMsgId, (void*)&b, sizeof(RecvInfo), 0) == -1)
	{
		return ERR_SND_MSG;
	}

	return SUCCESS;
}


int multiCastRecvMsg(RecvInfo *info)
{
	MsgBuf b;


	if(msgrcv(MulticastMsgId, (void*)&b, sizeof(RecvInfo), 1, 0) == -1)
	{
		return ERR_RECV_MSG;
	}

	*info = b.recvInfo;

	return SUCCESS;
}


void multiCastCloseMsg()
{
	if(MulticastMsgId != -1) {
		msgctl(MulticastMsgId, IPC_RMID, 0);
		MulticastMsgId = -1;
	}
}

