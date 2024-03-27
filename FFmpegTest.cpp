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

    return nRet;
}

int FFmpegTest::GetVideoDegree(int& nVideoDegree)
{
    int nRet = 0;
    AVDictionaryEntry *pRotaionTag = nullptr;
    AVFormatContext& pDecoderFormatCtx = *m_pFFmpegDecoder->m_pFormatCtx;

    pRotaionTag = av_dict_get(pDecoderFormatCtx.streams[m_pFFmpegDecoder->m_nVideoStreamIndex]->metadata, "rotate", nullptr, 0);

    if (pRotaionTag != nullptr) 
        nVideoDegree = atoi(pRotaionTag->value);

    return nRet;
}

int FFmpegTest::DecoingTest(const std::string& strInputUrl)
{
    int nRet = 0;
    const std::string strOutputWAVUrl = "test.wav";
    std::ofstream ofsWAVFile(strOutputWAVUrl);

    nRet = m_pFFmpegDecoder->OpenFile(strInputUrl);

    if(nRet == static_cast<int>(FD_RESULT::WARNING_FAIL_OPEN_INPUT) || nRet == static_cast<int>(FD_RESULT::WARNING_FAIL_READ_PACKET))
        return nRet;        

    switch (nRet)
    {
    case static_cast<int>(FD_RESULT::OK):
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegDecoder->OpenAudio();
        nRet = m_pFFmpegDecoder->DecodeVideo();
        nRet = m_pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case static_cast<int>(FD_RESULT::WARNING_NO_VIEDO_STREAM):
        nRet = m_pFFmpegDecoder->OpenAudio();
        nRet = m_pFFmpegDecoder->DecodeAudio(ofsWAVFile);
        break;
    case static_cast<int>(FD_RESULT::WARNING_NO_AUDIO_STREAM):
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegDecoder->DecodeVideo();
        break;
    default:
        std::cout << "nRet Value not in FD_RESULT:"<< nRet << std::endl;
        break;
    }
    WriteSizeWAVHeader(ofsWAVFile);

    return nRet;
}

int FFmpegTest::StartEncoding()
{
    int nRet = 0;
    int nFrameNumber = 0;
    
    while(true)
    {
        nRet = m_pFFmpegDecoder->DecodeVideoOneFrame(*m_pFrameData);
        if (nRet == static_cast<int>(FD_RESULT::WARNING_DECODER_END_FILE) || nFrameNumber == 500)
        {
            m_pFFmpegEncoder->FlushEncodeVideo(*m_pFrameData);
            std::cout << "Decoder & Encoder End.. nFrameNumber: " << nFrameNumber << std::endl;
            break;
        }
        nRet = m_pFFmpegEncoder->EncodeVideo(*m_pFrameData);

        nFrameNumber++;
        std::cout << "Decoding & Encoding: " << nFrameNumber << std::endl;

    }
    return nRet;
}

int FFmpegTest::EncoingTest(const std::string& strInputUrl)
{
    int nRet = 0;
    int VideoDegree = 0;
    nRet = m_pFFmpegDecoder->OpenFile(strInputUrl);
    const std::string strOutputEncoderUrl = "TestEncoder4.mp4";
    GetVideoDegree(VideoDegree);

    switch (nRet)
    {
    case static_cast<int>(FD_RESULT::OK):
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegEncoder->SetEncoder(VideoDegree, strOutputEncoderUrl);
        nRet= StartEncoding();
        break;
    case static_cast<int>(FD_RESULT::WARNING_NO_AUDIO_STREAM):
        nRet = m_pFFmpegDecoder->OpenVideo();
        nRet = m_pFFmpegEncoder->SetEncoder(VideoDegree, strOutputEncoderUrl);
        nRet = StartEncoding();
        break;
    default:
        break;
    }

    return nRet;
}

int main()
// int main(int argc, char *argv[])
{
    av_log_set_level(AV_LOG_ERROR);
    float fProcessDuration = 0.f;
    // const std::string strInputUrl = "/root/FFmpeg_test//TestVideo/terra.mp4";
    // const std::string strInputUrl = "/root/FFmpeg_test/TestVideo//TestVideo/output_mpeg.mp4";
    // const std::string strInputUrl = "/root/FFmpeg_test/TestVideo/cocovid_sv_convert_30_.mp4";
    const std::string strInputUrl = "/root/FFmpeg_test/TestVideo/phone1.mp4";
    
    FFmpegTest FFmpegTestObj;
    std::clock_t clockStartTime = std::clock();

    // FFmpegTestObj.DecoingTest(strInputUrl);
    FFmpegTestObj.EncoingTest(strInputUrl);

    std::clock_t clockEndTime = std::clock();
    fProcessDuration = (clockEndTime - clockStartTime) / CLOCKS_PER_SEC;
    std::cout << "ProcessDuration: " << fProcessDuration << "ms" << std::endl;

    return 0;
}