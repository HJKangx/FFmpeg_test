#include "ffmpegTest.h"

int main(int argc, char *argv[])
{
    int nRet = 0;
    float fProcessDuration = 0.f;
    // const std::string strInputUrl = "./TestVideo/terra.mp4";
    const std::string strInputUrl = "./TestVideo/output_mpeg.mp4";
    // const std::string strInputUrl = "./TestVideo/output_264.mp4";
    const std::string strOutputWAVUrl = "test.wav";
    std::ofstream ofsWAVFile(strOutputWAVUrl);

    FFmpegDecoder FFmpegDecoderObj;
 
    std::clock_t clockStartTime = std::clock();

    nRet = FFmpegDecoderObj.OpenFile(strInputUrl);
    
    std::cout << nRet << std::endl;
    
    switch (nRet)
    {
    case 0:
        nRet = FFmpegDecoderObj.OpenVideo();
        // nRet = FFmpegDecoderObj.OpenAudio();

        nRet = FFmpegDecoderObj.DecodeVideo();
        // nRet = FFmpegDecoderObj.DecodeAudio(ofsWAVFile);
        break;
    case -17:
        nRet = FFmpegDecoderObj.OpenAudio();
        nRet = FFmpegDecoderObj.DecodeAudio(ofsWAVFile);
        break;
    case -16:
        nRet = FFmpegDecoderObj.OpenVideo();
        nRet = FFmpegDecoderObj.DecodeVideo();
    default:
        break;
    }

    
    WriteSizeWAVHeader(ofsWAVFile);
    std::clock_t clockEndTime = std::clock();

    fProcessDuration = 1000.0 * (clockEndTime - clockStartTime) / CLOCKS_PER_SEC;
    std::cout << "ProcessDuration: " << fProcessDuration << "ms" << std::endl;

    return 0;
}

int WriteSizeWAVHeader(std::ofstream& ofsWAVFile)
{
    ofsWAVFile.seekp(0, std::ios::end);
    std::streampos fileSize = ofsWAVFile.tellp();

    int nRiffChunkSize = static_cast<int>(fileSize) - 8;
    int nDataChunkSize = static_cast<int>(fileSize) - 44; 

    ofsWAVFile.seekp(4);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nRiffChunkSize), 4);

    ofsWAVFile.seekp(40);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nDataChunkSize), 4);

    ofsWAVFile.close();
}