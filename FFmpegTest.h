#pragma once

#include "FFmpegDecoder.h"
#include "FFmpegEncoder.h"

class FFmpegTest
{
public: 
    FFmpegTest();

    ~FFmpegTest();

    int DecoingTest();
    int WriteSizeWAVHeader(std::ofstream& ofsWAVFile);
    
    std::shared_ptr<FFmpegDecoder> pFFmpegDecoder; 

};
