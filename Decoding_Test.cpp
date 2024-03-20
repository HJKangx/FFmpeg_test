#include "ffmpegDecode.h"

int main(int argc, char *argv[])
{
    int nRet = 0;
    double dProcessDuration = 0.0;
    const std::string strInputUrl = "terra.mp4";
    // const std::string strInputUrl = "output_mpeg.mp4";
    // const std::string strInputUrl = "output_264.mp4";

    FFmpegDecoder FFmpegDecoderObj;

    std::clock_t clockStartTime = std::clock();

    nRet = FFmpegDecoderObj.OpenFile(strInputUrl);


    switch (nRet)
    {
    case 0:
        nRet = FFmpegDecoderObj.OpenVideo();
        nRet = FFmpegDecoderObj.OpenAudio();
        nRet = FFmpegDecoderObj.DecodeVideo();
        break;
    case -17:
        nRet = FFmpegDecoderObj.OpenAudio();
        break;
    case -16:
        nRet = FFmpegDecoderObj.OpenVideo();
        nRet = FFmpegDecoderObj.DecodeVideo();

    default:
        break;
    }


    FFmpegDecoderObj.CloseFile(); 
    std::clock_t clockEndTime = std::clock();

    dProcessDuration = 1000.0 * (clockEndTime - clockStartTime) / CLOCKS_PER_SEC;
    std::cout << "ProcessDuration: " << dProcessDuration << "ms" << std::endl;


    return 0;
}