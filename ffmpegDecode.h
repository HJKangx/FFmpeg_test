#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <fmt/core.h>
#include "FFMPEG_DEFINE.h"

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
    FFmpegDecoder();

    ~FFmpegDecoder();
public:
    int OpenFile(const std::string& sInputFile);
    int CloseFile();
    int DecodeVideo();
    int DecodeAudio();
    int OpenVideo();
    int OpenAudio();
private:
    
    int ConvertRGBAVframe(AVFrame& pFrameYuv, AVFrame& pOutFrame);
    int BMPSave(AVFrame* pFrameRGB,  int width, int height);

    AVFormatContext* m_pFormatCtx;
    AVCodecContext* m_pVideoCodecCtx;
    AVCodecContext* m_pAudioCodecCtx;
    
    AVCodec* m_pVideoCodec;
    AVCodec* m_pAudioCodec;
    
    int m_nVideoStreamIndex;
    int m_nAudioStreamIndex;
    int m_nTotalFrameNumber;
};
