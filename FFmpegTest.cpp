#include "FFmpegTest.h"

FFmpegTest::FFmpegTest()
{
    m_pFFmpegDecoder = std::make_shared<FFmpegDecoder>();
    m_pFFmpegEncoder = std::make_shared<FFmpegEncoder>();
    m_pFrameData = std::make_shared<AVFrame>();
}

FFmpegTest::~FFmpegTest()
{
    std::cout << "End FFmpegTest" << std::endl;
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

int FFmpegTest::StartEncoding()
{
    int nRet = 0;
    int nFrameNumber = 0;
    const std::string strOutputEncoderUrl = "TestEncoder.h264";
    std::ofstream ofsH264File(strOutputEncoderUrl);

    while(true)
    {
        nRet = m_pFFmpegDecoder->DecodeVideoOneFrame(*m_pFrameData);
        if (nRet == -10)
        {
            // nRet = m_pFFmpegEncoder->EncodeVideo(*m_pFrameData, ofsH264File);
            std::cout << "Decoder & Encoder End.. nFrameNumber: " << nFrameNumber << std::endl;
            break;
        }
        nRet = m_pFFmpegEncoder->EncodeVideo(*m_pFrameData, ofsH264File);
        nFrameNumber++;


    }
    return nRet;
}

int FFmpegTest::DecoingTest(const std::string& strInputUrl)
{
    int nRet = 0;
    const std::string strOutputWAVUrl = "test.wav";
    std::ofstream ofsWAVFile(strOutputWAVUrl);

    nRet = m_pFFmpegDecoder->OpenFile(strInputUrl);

    switch (nRet)
    {
    case 0:
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegDecoder->OpenAudio();
        nRet = m_pFFmpegDecoder->DecodeVideo();
        nRet = m_pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case -17:
        nRet = m_pFFmpegDecoder->OpenAudio();
        nRet = m_pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case -16:
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegDecoder->DecodeVideo();
        break;
    default:
        break;
    }

    WriteSizeWAVHeader(ofsWAVFile);

    return nRet;
}

int FFmpegTest::EncoingTest(const std::string& strInputUrl)
{
    int nRet = 0;
    

    nRet = m_pFFmpegDecoder->OpenFile(strInputUrl);
    
    std::cout << nRet << std::endl;
    
    switch (nRet)
    {
    case 0:
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegEncoder->SetEncoder();
        nRet= StartEncoding();
        break;
    case -16:
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegEncoder->SetEncoder();
        nRet = StartEncoding();
        break;
    default:
        break;
    }

    return nRet;
}


int main(int argc, char *argv[])
{
    float fProcessDuration = 0.f;
    // const std::string strInputUrl = "./TestVideo/terra.mp4";
    const std::string strInputUrl = "./TestVideo/output_mpeg.mp4";
    // const std::string strInputUrl = "./TestVideo/output_264.mp4";
    
    FFmpegTest FFmpegTestObj;
    std::clock_t clockStartTime = std::clock();

    // FFmpegTestObj.DecoingTest(strInputUrl);
    FFmpegTestObj.EncoingTest(strInputUrl);

    std::clock_t clockEndTime = std::clock();
    std::cout << "ProcessDuration: " << fProcessDuration << "ms" << std::endl;
    fProcessDuration = 1000.0 * (clockEndTime - clockStartTime) / CLOCKS_PER_SEC;

    return 0;
}