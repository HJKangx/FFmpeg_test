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

class FFmpegEncoder
{
public: 
    FFmpegEncoder();

    ~FFmpegEncoder();
    
public:
    int EncodeVideo(AVFrame& pFrameYUV);

private:
    AVFormatContext* m_pFormatCtx;
    AVCodecContext* m_pEncoderCodecCtx;
    AVCodec* m_pEncoderCodec;
    


};
