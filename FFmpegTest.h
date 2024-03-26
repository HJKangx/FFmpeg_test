#pragma once

#include "FFmpegDecoder.h"
#include "FFmpegEncoder.h"

class FFmpegTest
{
public: 
    FFmpegTest();

    ~FFmpegTest();

    int DecoingTest(const std::string& strInputUrl);
    int EncoingTest(const std::string& strInputUrl);

private:
    int StartEncoding(std::ofstream& ofsOutputFile);
    int WriteSizeWAVHeader(std::ofstream& ofsWAVFile);
    
    std::shared_ptr<FFmpegDecoder> m_pFFmpegDecoder; 
    std::shared_ptr<FFmpegEncoder> m_pFFmpegEncoder; 
    std::shared_ptr<AVFrame> m_pFrameData;
};
