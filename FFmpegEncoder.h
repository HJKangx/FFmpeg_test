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
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

class FFmpegEncoder
{
public: 
    FFmpegEncoder();

    ~FFmpegEncoder();
    int SetEncoder(const std::string& ofsOutputFilePath);
    int EncodeVideo(const AVFrame& pFrameData);
    int FlushEncodeVideo(const AVFrame& pFrameData);
    int CloseEncoder();
    
    
private:

private:
    AVFormatContext* m_pOutputFormatCtx;
    AVOutputFormat* m_pOutputFormat;
    AVCodecContext* m_pEncoderCodecCtx;
    AVCodec* m_pEncoderCodec;
    AVStream* m_pVideoStream;

    int m_nEncoderCount;
};
