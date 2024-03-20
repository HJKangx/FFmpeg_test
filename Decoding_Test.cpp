#include "ffmpegDecode.h"

int main(int argc, char *argv[])
{
    int nRet = 0;
    const std::string strInputUrl = "terra.mp4";
    // const std::string strInputUrl = "output_mpeg.mp4";
    // const std::string strInputUrl = "output_264.mp4";

    FFmpegDecoder FFmpegDecoderObj;
    nRet = FFmpegDecoderObj.OpenFile(strInputUrl);

    if (nRet == 0)
    {
        nRet = FFmpegDecoderObj.OpenVideo();
        nRet = FFmpegDecoderObj.OpenAudio();
        nRet = FFmpegDecoderObj.DecodeVideo();
    }
    else if (nRet == -16)
    {
        nRet = FFmpegDecoderObj.OpenVideo();
        nRet = FFmpegDecoderObj.OpenAudio();
        nRet = FFmpegDecoderObj.DecodeVideo();
    }


    FFmpegDecoderObj.CloseFile(); 

    return 0;
}