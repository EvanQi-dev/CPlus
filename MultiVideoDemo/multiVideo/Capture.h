#pragma once
#include <QObject>
#include <QThread>

#define USE_DSHOW	   0
#define IN_FRAME_RATE  3
#define OUT_BITRATE    500000
#define OUT_W          1152
#define OUT_H          648
#define OUT_FRAME_RATE 3
#define OUT_GOP_SIZE   50

class MultiCast;

class Capture : public QThread
{
	Q_OBJECT

public:
	enum RecordType {
		SCREEN = 0,
		PICTURE
	};

	enum ErrCode {
		SUCCESS = 0,
		ERR_IN_SIZE,
		ERR_GDIGRAB,
		ERR_IN_FIND_STREAM,
		ERR_IN_FIND_DECODER,
		ERR_IN_OPEN_CODEC,
		ERR_IN_ALLOC_FRAME,
		ERR_IN_ALLOC_PACKET,
		ERR_IN_DECODE,
		ERR_OUT_FIND_ENCODER,
		ERR_OUT_ALLOC_OUTCONTEXT,
		ERR_OUT_OPEN_CODEC,
		ERR_OUT_OPEN_FILE,
		ERR_OUT_ALLOC_FRAME,
		ERR_OUT_ALLOC_BUFFER,
		ERR_OUT_ALLOC_SWSCONTEXT,
		ERR_OUT_ENCODE,
	};

	Capture();
	~Capture();

	void setInputFrameRate(int rate);
	void setScrCaptureRect(int x, int y, int w, int h);
	void setOutSize(int w, int h);

	void showDshowDevice();
	void showAvfoundationDevice();

	void startRecord();	// Æô¶¯Â¼ÖÆ
	void stopRecord();	// ¹Ø±ÕÂ¼ÖÆ

	int recordScreen();	// Â¼ÆÁÄ»
	int recordPicture();// Â¼Í¼Æ¬

signals:
	void sigRecordStop(int exitCode);

protected:
	virtual void run(void);

private:
	int recordType = SCREEN;

	int inFrameRate = IN_FRAME_RATE;
	int inScrX=0, inScrY=0, inScrW=0, inScrH=0;

	int outBitRate = OUT_BITRATE;
	int outW = OUT_W;
	int outH = OUT_H;
	int outFrameRate = OUT_FRAME_RATE;
	int outGopSize = OUT_GOP_SIZE;

	int errCode = SUCCESS;

	bool runFlag = false;

	MultiCast *multiCast = nullptr;
};

