#include "FFmpegTest.h"

FFmpegTest::FFmpegTest()
{
    pFFmpegDecoder = std::make_shared<FFmpegDecoder>();
}

FFmpegTest::~FFmpegTest()
{
    std::cout << "End FFmpegTest" << std::endl;
}

int FFmpegTest::DecoingTest()
{
    int nRet = 0;
    float fProcessDuration = 0.f;
    // const std::string strInputUrl = "./TestVideo/terra.mp4";
    const std::string strInputUrl = "./TestVideo/output_mpeg.mp4";
    // const std::string strInputUrl = "./TestVideo/output_264.mp4";
    const std::string strOutputWAVUrl = "test.wav";
    std::ofstream ofsWAVFile(strOutputWAVUrl);

    std::clock_t clockStartTime = std::clock();

    nRet = pFFmpegDecoder->OpenFile(strInputUrl);
    
    std::cout << nRet << std::endl;
    
    switch (nRet)
    {
    case 0:
        nRet = pFFmpegDecoder->OpenVideo();
        // nRet = pFFmpegDecoder->OpenAudio();

        nRet = pFFmpegDecoder->DecodeVideo();
        // nRet = pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case -17:
        nRet = pFFmpegDecoder->OpenAudio();
        nRet = pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case -16:
        nRet = pFFmpegDecoder->OpenVideo();
        nRet = pFFmpegDecoder->DecodeVideo();
    default:
        break;
    }

    
    WriteSizeWAVHeader(ofsWAVFile);
    std::clock_t clockEndTime = std::clock();

    fProcessDuration = 1000.0 * (clockEndTime - clockStartTime) / CLOCKS_PER_SEC;
    std::cout << "ProcessDuration: " << fProcessDuration << "ms" << std::endl;

    return nRet;
}

int FFmpegTest::WriteSizeWAVHeader(std::ofstream& ofsWAVFile)
{
    int nRet = 0;
    ofsWAVFile.seekp(0, std::ios::end);
    std::streampos fileSize = ofsWAVFile.tellp();

    int nRiffChunkSize = static_cast<int>(fileSize) - 8;
    int nDataChunkSize = static_cast<int>(fileSize) - 44; 

    ofsWAVFile.seekp(4);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nRiffChunkSize), 4);
    ofsWAVFile.seekp(40);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nDataChunkSize), 4);

    ofsWAVFile.close();

    return nRet;
}

int main(int argc, char *argv[])
{
    FFmpegTest FFmpegTestObj;
    FFmpegTestObj.DecoingTest();
        
    return 0;
}