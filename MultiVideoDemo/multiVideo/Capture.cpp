
// 纪念 雷霄骅 Lei Xiaohua

#include <stdio.h>
#include <stdlib.h>
#include "Capture.h"
#include "MultiCast.h"
#include "../include/HLog.h"
#include <QString>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
};

Capture::Capture() : QThread()
{
	av_register_all();
	avformat_network_init();
	avdevice_register_all();

	multiCast = new MultiCast();
}

Capture::~Capture()
{
	delete multiCast;
}

// Show direct show device
// TODO 信息放到数组中
void Capture::showDshowDevice()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("dshow");
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);

	avformat_free_context(pFormatCtx);
}

// Show AVFoundation device
// TODO 信息放到数组中
void Capture::showAvfoundationDevice()
{
	AVFormatContext *pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat *iformat = av_find_input_format("avfoundation");
	avformat_open_input(&pFormatCtx, "", iformat, &options);

	avformat_free_context(pFormatCtx);
}

int Capture::recordScreen()
{
	int rc = SUCCESS;
	if (inScrW == 0 || inScrH == 0) {
		runFlag = false;
		return ERR_IN_SIZE;
	}

	AVFormatContext *pFormatCtx;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	unsigned int i;
	int videoindex;

	//Assign sth to the format context.
	pFormatCtx = avformat_alloc_context();

	//Windows
#if USE_DSHOW
	//Use dshow
	//
	//Need to Install screen-capture-recorder
	//screen-capture-recorder
	//Website: http://sourceforge.net/projects/screencapturer/
	//
	AVInputFormat *ifmt = av_find_input_format("dshow");
	//if(avformat_open_input(&pFormatCtx,"video=screen-capture-recorder",ifmt,NULL)!=0)
	if (avformat_open_input(&pFormatCtx, "video=UScreenCapture", ifmt, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
#else
	//Use gdigrab
	char buf[20];
	AVDictionary* options = NULL;

	//Set input options
	//grabbing frame rate
	_itoa(inFrameRate, buf, 10);
	av_dict_set(&options, "framerate", buf, 0);
	//The distance from the left edge of the screen or desktop
	_itoa(inScrX, buf, 10);
	av_dict_set(&options, "offset_x", buf, 0);
	//The distance from the top edge of the screen or desktop
	_itoa(inScrY, buf, 10);
	av_dict_set(&options, "offset_y", buf, 0);
	//Video frame size. The default is to capture the full screen
	sprintf(buf, "%dx%d", inScrW, inScrH);
	av_dict_set(&options, "video_size", buf, 0);

	// 打开input
	AVInputFormat *ifmt = av_find_input_format("gdigrab");
	if (avformat_open_input(&pFormatCtx, "desktop", ifmt, &options) != 0){
		rc = ERR_GDIGRAB;
		goto ret_0;
	}
#endif

	//Find video stream.
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		rc = ERR_IN_FIND_STREAM;
		goto ret_1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	if (videoindex == -1)
	{
		rc = ERR_IN_FIND_STREAM;
		goto ret_1;
	}

	// 获取decoder
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		rc = ERR_IN_FIND_DECODER;
		goto ret_1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		rc = ERR_IN_OPEN_CODEC;
		goto ret_1;
	}

	//This is where you control the format(through frames).
	int ret, got_picture;
	AVFrame *pFrame;
	pFrame = av_frame_alloc();
	if (pFrame == NULL) {
		rc = ERR_IN_ALLOC_FRAME;
		goto ret_2;
	}

	//Try to init the packet here.
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	if (packet == NULL) {
		rc = ERR_IN_ALLOC_PACKET;
		goto ret_3;
	}
	av_init_packet(packet);

	//Output Information
	av_dump_format(pFormatCtx, 0, NULL, 0);

	const char * filename = "test.mpg";
	int codec_id = AV_CODEC_ID_MPEG4;	//AV_CODEC_ID_MPEG1VIDEO;

	AVCodec *outCodec;
	AVCodecContext *outContext = NULL;
	int got_output;
	FILE *f;
	AVPacket pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	printf("Encode video file %s\n", filename);

	//Find the mpeg4 video encoder
	outCodec = avcodec_find_encoder((AVCodecID)codec_id);
	if (!outCodec) {
		fprintf(stderr, "Codec not found\n");
		rc = ERR_OUT_FIND_ENCODER;
		goto ret_4;
	}

	outContext = avcodec_alloc_context3(outCodec);
	if (!outContext) {
		fprintf(stderr, "Could not allocate video codec context\n");
		rc = ERR_OUT_ALLOC_OUTCONTEXT;
		goto ret_4;
	}

	//put sample parameters
	outContext->bit_rate = outBitRate;
	//outContext->flags |= CODEC_FLAG_QSCALE;
	//outContext->rc_min_rate = 10000;
	//outContext->rc_max_rate = 500000;

	//Resolution must be a multiple of two
	outContext->width = outW;
	outContext->height = outH;

	//Frames per second
	outContext->time_base.num = 1;
	outContext->time_base.den = outFrameRate;	// FRAME_RATE

	/*
	* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/

	//设置多少帧插入1个I帧，I帧越少，视频越小 
	outContext->gop_size = outGopSize;
	outContext->max_b_frames = 1;
	outContext->pix_fmt = AV_PIX_FMT_YUV420P;

	//av_opt_set(outContext->priv_data, "preset", "slow", 0);

	//Open it
	if (avcodec_open2(outContext, outCodec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		rc = ERR_OUT_OPEN_CODEC;
		goto ret_5;
	}

	f = fopen(filename, "wb");
	if (!f) {
		fprintf(stderr, "Could not open %s\n", filename);
		rc = ERR_OUT_OPEN_FILE;
		goto ret_5;
	}

	AVFrame *outframe = av_frame_alloc();
	if (outframe == NULL) {
		rc = ERR_OUT_ALLOC_FRAME;
		goto ret_6;
	}

	int nbytes = avpicture_get_size(outContext->pix_fmt, outContext->width, outContext->height);
	uint8_t* outbuffer = (uint8_t*)av_malloc(nbytes);
	if (outbuffer == NULL) {
		rc = ERR_OUT_ALLOC_BUFFER;
		goto ret_7;
	}

	//Associate the frame to the allocated buffer.
	avpicture_fill((AVPicture*)outframe, outbuffer, AV_PIX_FMT_YUV420P, outContext->width, outContext->height);

	SwsContext* swsCtx;
	swsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, outContext->width, outContext->height, outContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (swsCtx == NULL) {
		rc = ERR_OUT_ALLOC_SWSCONTEXT;
		goto ret_8;
	}

	int offset = 0;

	//Here we start pulling packets from the specified format context.
	while (runFlag && av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoindex)
		{
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				rc = ERR_IN_DECODE;
				break;
			}

			if (got_picture)
			{
				ret = sws_scale(swsCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, outframe->data, outframe->linesize);
				if (ret <= 0)
					continue;

				av_init_packet(&pkt);
				pkt.data = NULL;    // packet data will be allocated by the encoder
				pkt.size = 0;
				ret = avcodec_encode_video2(outContext, &pkt, outframe, &got_output);
				if (ret < 0) {
					fprintf(stderr, "Error encoding frame\n");
					rc = ERR_OUT_ENCODE;
					break;
				}

				if (got_output) {
					printf("Write frame %3d (size=%5d)\n", i, pkt.size);
					fwrite(pkt.data, 1, pkt.size, f);
					offset += pkt.size;
					multiCast->enqueueBlock(pkt.data, pkt.size);
					av_packet_unref(&pkt);
				}
			}
		}

		//av_free_packet(packet);
		av_packet_unref(packet);
	} //THE LOOP TO PULL PACKETS FROM THE FORMAT CONTEXT ENDS HERE.

	/* get the delayed frames */
	if (rc == SUCCESS) {
		for (got_output = 1; got_output; i++) {
			ret = avcodec_encode_video2(outContext, &pkt, NULL, &got_output);
			if (ret < 0) {
				fprintf(stderr, "Error encoding frame\n");
				rc = ERR_OUT_ENCODE;
				break;
			}

			if (got_output) {
				printf("Write frame %3d (size=%5d)\n", i, pkt.size);
				multiCast->enqueueBlock(pkt.data, pkt.size);
				fwrite(pkt.data, 1, pkt.size, f);
				av_packet_unref(&pkt);
			}
		}
	}

	/* add sequence end code to have a real mpeg file */
	fwrite(endcode, 1, sizeof(endcode), f);

	sws_freeContext(swsCtx);
ret_8:
	av_free(outbuffer);
ret_7:
	av_frame_free(&outframe);
ret_6:
	fclose(f);
ret_5:
	avcodec_free_context(&outContext);
ret_4:
	av_packet_unref(packet);
ret_3:
	av_frame_free(&pFrame);
ret_2:
	avcodec_close(pCodecCtx);
ret_1:
	av_dict_free(&options);
	avformat_close_input(&pFormatCtx);
ret_0:
	avformat_free_context(pFormatCtx);

	runFlag = false;

	multiCast->stopMultiCast();

	emit sigRecordStop(rc);

	return rc;
}

int Capture::recordPicture()
{
	return SUCCESS;
}

void Capture::run()
{
	switch (recordType) {
	case SCREEN:
		recordScreen();
		break;
	case PICTURE:
		recordPicture();
		break;
	}
}

void Capture::startRecord()
{
	if (runFlag == false) {
		multiCast->startMultiCast();

		runFlag = true;
		start();
	}
}

void Capture::stopRecord()
{
	runFlag = false;
}

void Capture::setScrCaptureRect(int x, int y, int w, int h)
{
	inScrX = x;
	inScrY = y;
	inScrW = w;
	inScrH = h;
}

void Capture::setOutSize(int w, int h)
{
	outW = w;
	outH = h;
}
