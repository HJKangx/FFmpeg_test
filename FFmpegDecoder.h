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
    int OpenVideo();
    int OpenAudio();    
    int DecodeVideo();
    int DecodeAudio(std::ofstream& ofsWAVFile);
    std::shared_ptr<FFmpegDecoder> m_pFFmpegDecoder;
    
private:
    int ConvertRGBAVframe(AVFrame& pFrameYuv, AVFrame& pOutFrame);
    int SaveBMP(AVFrame* pFrameRGB,  int width, int height);
    int MakeWAVHeader(std::ofstream& ofsWAVFile);
    int GetAudioBitDepth(short& nBitDepth);
    int SaveWAV(AVFrame* pFrameAudio, std::ofstream& ofsWAVFile);
    int CloseFile();

    AVFormatContext* m_pFormatCtx;
    AVCodecContext* m_pVideoCodecCtx;
    AVCodecContext* m_pAudioCodecCtx;
    AVCodec* m_pVideoCodec;
    AVCodec* m_pAudioCodec;
    
    int m_nVideoStreamIndex;
    int m_nAudioStreamIndex;
    int m_nTotalFrameNumber;
    float m_fInputDuration;

};
