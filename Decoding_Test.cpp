#include "ffmpegDecode.h"


int main(int argc, char *argv[])
{
    // const std::string strInputUrl = "terra.mp4";
    int nRet = 0;
    const std::string strInputUrl = "output_mpeg.mp4";

    FFmpegDecoder decoder;
    nRet = decoder.OpenFile(strInputUrl);

    if (nRet != -15 && nRet != -17)
        nRet = decoder.DecodeVideo();
    // decoder.CloseFile(); 

    return 0;
}

//close
//avformat_close_input(&m_pFormatCtx);
