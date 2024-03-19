#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}


class FFmpegDecoder
{
public: 
    FFmpegDecoder() : pFormatCtx(NULL) {;}


public:
    int OpenFile(const std::string& sInputFile);

    int CloseFile();

    int GetNextFrame(AVFrame& pFrame);

    int GetWidth();
    int GetHeight();
    int DecodeVideo();


private:
    int OpenVideo();
    int OpenAudio();
    int CloseVideo();
    int CloseAudio();

    int DecodeAudio(int nStreamIndex, const AVPacket *avpkt, uint8_t* pOutBuffer, size_t nOutBufferSize);
    
    int GetRGBAframe(AVFrame& pFrameYuv, AVFrame* pOutFrame);

    int BMPSave(AVFrame* pFrameRGB,  int width, int height);

    AVFormatContext* pFormatCtx;
    AVCodecContext* pVideoCodecCtx;
    AVCodecContext* pAudioCodecCtx;
    
    AVCodec* pVideoCodec;
    AVCodec* pAudioCodec;
    
    int nVideoStreamIndex;
    int nAudioStreamIndex;

    double dVedeoFramePerSecond;
    double dVideoBaseTime;
    double dAudioBaseTime;
    

    struct SwsContext* pImgConvertCtx;
};
