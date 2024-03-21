#include "ffmpegDecode.h"

FFmpegDecoder::FFmpegDecoder()
{
    m_pFormatCtx = nullptr;
    m_pVideoCodecCtx = nullptr;
    m_pAudioCodecCtx = nullptr;

    m_pVideoCodec = nullptr;
    m_pAudioCodec = nullptr;

    m_nVideoStreamIndex = -1;
    m_nAudioStreamIndex = -1;
    m_nTotalFrameNumber = 0;
    m_fInputDuration = 0.f;
}

FFmpegDecoder::~FFmpegDecoder()
{
    CloseFile();
    std::cout << ("End FFmpegDecoder.") << std::endl;
}

int FFmpegDecoder::OpenFile(const std::string& strInputUrl)
{
    int nRet = 0;
    int nInputWidth = 0;
    int nInputHeight = 0;
    float fFps = 0.f;

    std::string strVideoCodecName = "None";
    std::string strAudioCodecName = "None";

    std::cout << "OpenFile: " << strInputUrl << std::endl;

    if (avformat_open_input(&m_pFormatCtx, strInputUrl.c_str(), nullptr, nullptr) < 0)
	{
        std::cerr << "Can't Open Stream & Read Header in " << strInputUrl << std::endl;
		return false;
	}
    if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
	{
        std::cerr << "Can't Read Packets of " << strInputUrl << std::endl;
		return false;
	}
    // av_dump_format(m_pFormatCtx, 0, 0, 0);
    for(int i = 0; i < m_pFormatCtx->nb_streams; i++)
    {
        if(m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_nVideoStreamIndex = i;
            std::cout << "Video stream idx: "<< m_nVideoStreamIndex << std::endl;

            AVRational avrAvgFrameRate = m_pFormatCtx->streams[m_nVideoStreamIndex]->avg_frame_rate;
            fFps = static_cast<float>(avrAvgFrameRate.num) / static_cast<float>(avrAvgFrameRate.den);

			m_pVideoCodec = avcodec_find_decoder(m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->codec_id);
            nInputWidth = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->width;
            nInputHeight = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->height;

            m_fInputDuration = static_cast<float>(m_pFormatCtx->duration) / static_cast<float>(AV_TIME_BASE);
            m_nTotalFrameNumber = static_cast<int>(m_fInputDuration * fFps);
            strVideoCodecName = m_pVideoCodec->name;
        }
        else if(m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_nAudioStreamIndex = i;
            std::cout << "Audio stream idx: "<< m_nAudioStreamIndex << std::endl;

			m_pAudioCodec = avcodec_find_decoder(m_pFormatCtx->streams[m_nAudioStreamIndex]->codecpar->codec_id);
            strAudioCodecName = m_pAudioCodec->name;
        }
    }

    fmt::print("Video Duration - {} seconds, {} fps, {} frame\nVideo Info - Width: {}, Height: {}, Video codec: {}, Audio codec: {}\n",
           m_fInputDuration, fFps, m_nTotalFrameNumber, nInputWidth, nInputHeight, strVideoCodecName, strAudioCodecName);

    if (m_nVideoStreamIndex == -1 && m_nAudioStreamIndex == -1)
    {
        nRet = static_cast<int>(FD_RESULT::ERROR_NO_AUDIO_AND_VIEDO_STREAM);
        
        return nRet;
    }
    else if (m_nVideoStreamIndex == -1)
    {
        std::cerr << "No Viedo stream in " << strInputUrl << std::endl;
        nRet = static_cast<int>(FD_RESULT::WARNING_NO_VIEDO_STREAM);
    }
    else if (m_nAudioStreamIndex == -1)
    {
        std::cerr << "No Audio stream in " << strInputUrl << std::endl;
        nRet = static_cast<int>(FD_RESULT::WARNING_NO_AUDIO_STREAM);
    }

    return nRet;
}

int FFmpegDecoder::OpenVideo()
{
    int nRet = 0;
   
    if (m_pVideoCodec != nullptr && m_pFormatCtx != nullptr)
    {
        m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
        nRet = avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar);

        if (m_pVideoCodecCtx != nullptr && m_pVideoCodecCtx->codec_id != 0 && nRet >= 0)
        {
            nRet = (avcodec_open2)(m_pVideoCodecCtx, m_pVideoCodec, nullptr);

            if (nRet != 0)
            {
                nRet = -1;
                std::cout << "Fail avcodec_open2 Video" << std::endl;        
                return nRet;
            } 
        }
        else
        {
            nRet = -1;
            std::cout << "Fail Allocate an AVCodecContext or Fail Fill the Codec Context" << std::endl;    
            return nRet;
        }
    }
    return nRet;
}

int FFmpegDecoder::OpenAudio()
{
    int nRet = 0;

    if (m_pAudioCodec != nullptr && m_pFormatCtx != nullptr)
    {
        m_pAudioCodecCtx = avcodec_alloc_context3(m_pAudioCodec);
        nRet = avcodec_parameters_to_context(m_pAudioCodecCtx, m_pFormatCtx->streams[m_nAudioStreamIndex]->codecpar);

        if (m_pAudioCodecCtx != nullptr && m_pAudioCodecCtx->codec_id != 0 && nRet >= 0)
        {
            nRet = (avcodec_open2)(m_pAudioCodecCtx, m_pAudioCodec, nullptr);
            

            if (nRet != 0)
            {
                nRet = -1;
                std::cout << "Fail avcodec_open2 Audio" << std::endl;        
                return nRet;
            } 
        }
    }
    return nRet;
}

int FFmpegDecoder::DecodeVideo()
{
    int nRet = 0;
    int nFrameNumber = 0;
    int nPts = 0;
    
    AVFrame* pFrameYUV = av_frame_alloc();
    AVFrame* pFrameRGB = av_frame_alloc();
    AVPacket* pPacket = av_packet_alloc();

    // while (av_read_frame(m_pFormatCtx, pPacket) >= 0)
    while (true)
    { 
        nRet = av_read_frame(m_pFormatCtx, pPacket);

        if (nRet < 0)
        {
            // AVERROR(EAGAIN) -541478725
            if (nRet == AVERROR(EAGAIN))
            {
                nRet = 0;
                return nRet;
            }
            return nRet;
        }
        if(pPacket->stream_index == m_nVideoStreamIndex)
        {
            // nPts = (pPacket->dts != AV_NOPTS_VALUE) ? pPacket->dts : 0;
            // pPacket->pts = nPts;
            nRet = avcodec_send_packet(m_pVideoCodecCtx, pPacket);
            av_packet_unref(pPacket);
            // std::cout << "avcodec_send_packet nRet: " << nRet << " " << pPacket->dts << " / pPacket /" << pPacket->pts << std::endl;

            if (nRet == 0)
            {
                nRet = avcodec_receive_frame(m_pVideoCodecCtx, pFrameYUV);

                nFrameNumber++;
                std::cout << "Read FrameNumber: " << nFrameNumber << std::endl;

                if (nRet == 0)
                {
                    nRet = ConvertRGBAVframe(*pFrameYUV, *pFrameRGB);
                    nRet = SaveBMP(pFrameRGB, pFrameRGB->width, pFrameRGB->height);
                    av_frame_unref(pFrameYUV);
                    av_frame_unref(pFrameRGB);
                }
            }
            else
            {
                std::cerr << "avcodec_send_packet - ErrorCode:" << nRet << std::endl;
            }
        }
    }
    return nRet;
}

int FFmpegDecoder::DecodeAudio(std::ofstream& ofsWAVFile)
{
    int nRet = 0;
    int nFrameNumber  = 0;
    
    AVFrame* pFrameAudio = av_frame_alloc();
    AVPacket* pPacket = av_packet_alloc();

    MakeWAVHeader(ofsWAVFile);

    while (true)
    { 
        nRet = av_read_frame(m_pFormatCtx, pPacket);

        if (nRet < 0)
        {
            // AVERROR(EAGAIN) -541478725
            if (nRet == AVERROR(EAGAIN))
            {
                nRet = 0;
                return nRet;
            }
            return nRet;
        }

        if(pPacket->stream_index == m_nAudioStreamIndex)
        {

            nRet = avcodec_send_packet(m_pAudioCodecCtx, pPacket);

            av_packet_unref(pPacket);
            if (nRet == 0)
            {
                nRet = avcodec_receive_frame(m_pAudioCodecCtx, pFrameAudio);

                nFrameNumber++;
                std::cout << "Read Audio FrameNumber: " << nFrameNumber << std::endl;
                
                if (nRet == 0)
                {
                    SaveWAV(pFrameAudio, ofsWAVFile);
                    av_frame_unref(pFrameAudio);
                }
            }
        }
    }
    return nRet;
}


int FFmpegDecoder::ConvertRGBAVframe(AVFrame& pFrameYUV, AVFrame& pOutFrame)
{
    int nRet = 0;

    struct SwsContext *pSwsCtx = nullptr;
    int nInputWidth = pFrameYUV.width; 
    int nInputHeight = pFrameYUV.height;
    int nOutputWidth = pFrameYUV.width;
    int nOutputHeight = pFrameYUV.height; 

    pSwsCtx = sws_getContext(nInputWidth, nInputHeight, AV_PIX_FMT_YUV420P,
                    nOutputWidth, nOutputHeight, AV_PIX_FMT_BGR24,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);

    pOutFrame.format = AV_PIX_FMT_BGR24;
    pOutFrame.width  = nOutputWidth;
    pOutFrame.height = nOutputHeight;

    uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, nOutputWidth, nOutputHeight, 1));
    av_image_fill_arrays(pOutFrame.data, pOutFrame.linesize, out_buffer, AV_PIX_FMT_RGB24, nOutputWidth, nOutputHeight, 1);

    sws_scale(pSwsCtx, (const uint8_t* const*)pFrameYUV.data, pFrameYUV.linesize, 0, 
                        nInputHeight, pOutFrame.data, pOutFrame.linesize);

    return nRet;
}

int FFmpegDecoder::SaveBMP(AVFrame* pFrameRGB, int nWidth, int nHeight)
{
    int nFileSize = 54 + 3 * nWidth * nHeight;
    std::ofstream ofsBMPFile("FFmpeg_test.bmp");

    if (ofsBMPFile.is_open())
    {
        std::vector<unsigned char> vecFileHeader(14, 0);
        std::vector<unsigned char> vecInfoHeader(40, 0);

        vecFileHeader[0] = 'B'; vecFileHeader[1] = 'M';
        vecFileHeader[2] = nFileSize;
        vecFileHeader[3] = nFileSize >> 8;
        vecFileHeader[4] = nFileSize >> 16;
        vecFileHeader[5] = nFileSize >> 24;
        vecFileHeader[10] = 54;
        vecInfoHeader[0] = 40;
        vecInfoHeader[4] = nWidth;
        vecInfoHeader[5] = nWidth >> 8;
        vecInfoHeader[6] = nWidth >> 16;
        vecInfoHeader[7] = nWidth >> 24;
        vecInfoHeader[8] = nHeight;
        vecInfoHeader[9] = nHeight >> 8;
        vecInfoHeader[10] = nHeight >> 16;
        vecInfoHeader[11] = nHeight >> 24;
        vecInfoHeader[12] =  1; 
        vecInfoHeader[14] = 24;

        ofsBMPFile.write(reinterpret_cast<const char*>(vecFileHeader.data()), vecFileHeader.size());
        ofsBMPFile.write(reinterpret_cast<const char*>(vecInfoHeader.data()), vecInfoHeader.size());

        for (int i = nHeight - 1; i >= 0; i--) 
            ofsBMPFile.write(reinterpret_cast<const char*>(pFrameRGB->data[0] + i * pFrameRGB->linesize[0]), nWidth * 3);
    }
    else 
        std::cerr << "Failed to open file for writing" << std::endl;

    ofsBMPFile.close();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Save File" << std::endl;

    return 0;
}

int FFmpegDecoder::GetAudioBitDepth(short& nBitDepth)
{
    int nRet = 0;
    AVSampleFormat sampleFmt = m_pAudioCodecCtx->sample_fmt;

    switch (sampleFmt) 
    {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            nBitDepth = 8;
            break;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            nBitDepth = 16;
            break;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            nBitDepth = 32;
            break;
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_DBLP:
            nBitDepth = 64;
            break;
        default:
            std::cerr << "No sample_fmt" << std::endl;
            nBitDepth = -1;
    }
    return nRet;
}

int FFmpegDecoder::MakeWAVHeader(std::ofstream& ofsWAVFile)
{
    int nRet = 0;
    int nSampleRate = m_pAudioCodecCtx->sample_rate;
    short sBitsPerSample = 0;
    short sChannels = m_pAudioCodecCtx->channels;
    GetAudioBitDepth(sBitsPerSample);

    int byteRate = nSampleRate * sChannels * sBitsPerSample / 8;
    short blockAlign = sChannels * sBitsPerSample / 8; 

    ofsWAVFile.write("RIFF", 4);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nSampleRate), 4);
    ofsWAVFile.write("WAVE", 4);

    ofsWAVFile.write("fmt ", 4);
    int fmtChunkSize = 16;
    ofsWAVFile.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
    short audioFormat = 1;
    ofsWAVFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    ofsWAVFile.write(reinterpret_cast<const char*>(&sChannels), 2);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nSampleRate), 4); //
    ofsWAVFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    ofsWAVFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    ofsWAVFile.write(reinterpret_cast<const char*>(&sBitsPerSample), 2);

    ofsWAVFile.write("data", 4);
    ofsWAVFile.write(reinterpret_cast<const char*>(&nSampleRate), 4); //

    return nRet;
}

int FFmpegDecoder::SaveWAV(AVFrame* pFrameAudio, std::ofstream& ofsWAVFile)
{
    int nRet = 0;
    const int data_size = av_get_bytes_per_sample(m_pAudioCodecCtx->sample_fmt);

    for (int i = 0; i < pFrameAudio->nb_samples; ++i) 
    {
        for (int ch = 0; ch < m_pAudioCodecCtx->channels; ++ch) {
            const uint8_t* buf = pFrameAudio->data[ch] + data_size * i;

            ofsWAVFile.write(reinterpret_cast<const char*>(buf), data_size);
        }
    }

    
    // ofsWAVFile.close();
    return nRet;
}


int FFmpegDecoder::CloseFile()
{
    int nRet = 0;

    avformat_close_input(&m_pFormatCtx);
    avcodec_close(m_pVideoCodecCtx);
    avcodec_close(m_pAudioCodecCtx);
    m_pVideoCodecCtx = nullptr;
    m_pAudioCodecCtx = nullptr;
    m_pVideoCodec = nullptr;
    m_pAudioCodec = nullptr;
    m_nVideoStreamIndex = -1;
    m_nAudioStreamIndex = -1;

    return nRet;
}
