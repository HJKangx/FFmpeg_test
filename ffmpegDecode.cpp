#include "ffmpegDecode.h"
extern "C"

FFmpegDecoder::FFmpegDecoder()
{
    m_pFormatCtx = nullptr;
    m_pVideoCodecCtx = nullptr;
    m_pAudioCodecCtx = nullptr;

    m_pVideoCodec = nullptr;
    m_pAudioCodec = nullptr;

    pImgConvertCtx = nullptr;

    m_nVideoStreamIndex = -1;
    m_nAudioStreamIndex = -1;
}

FFmpegDecoder::~FFmpegDecoder()
{
    std::cout << ("End FFmpegDecoder.") << std::endl;
}

int FFmpegDecoder::OpenFile(const std::string& strInputUrl)
{
    int nRet = 0;
    int nInputDuration = 0;
    int nTotalFrame = 0;
    int nInputWidth = 0;
    int nInputHeight = 0;
    float fFps = 0.f;

    std::string stdVideoCodec = "None";
    std::string stdAudioCodec = "None";

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
            std::cout << "Video stream idx : "<< m_nVideoStreamIndex << std::endl;

            AVRational avrFps = m_pFormatCtx->streams[m_nVideoStreamIndex]->avg_frame_rate;
            fFps = static_cast<float>(avrFps.num) / static_cast<float>(avrFps.den);

			m_pVideoCodec = avcodec_find_decoder(m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->codec_id);
            nInputWidth = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->width;
            nInputHeight = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->height;

            nInputDuration = m_pFormatCtx->duration / AV_TIME_BASE;
            nTotalFrame = nInputDuration * fFps;
            stdVideoCodec = m_pVideoCodec->name;
        }
        else if(m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            m_nAudioStreamIndex = i;
            std::cout << "Audio stream idx: "<< m_nAudioStreamIndex << std::endl;

			m_pAudioCodec = avcodec_find_decoder(m_pFormatCtx->streams[m_nAudioStreamIndex]->codecpar->codec_id);
            stdAudioCodec = m_pAudioCodec->name;
        }
    }

    if (m_nVideoStreamIndex == -1 && m_nAudioStreamIndex == -1)
    {
        std::cerr << "No Viedo and Audio stream in " << strInputUrl << std::endl;
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

    std::cout << "Video Duration - " << nInputDuration << " seconds, " << fFps << "fps, " 
                << nTotalFrame << " frame" << std::endl;

    std::cout << "Video Info - " << "Width: " << nInputWidth << ", Height: " << nInputHeight << ", video codec: " << stdVideoCodec 
                << ", audio codec: " << stdAudioCodec << std::endl;
                
    bool bCheckVideo = OpenVideo();

    return nRet;
}

int FFmpegDecoder::OpenVideo()
{
    int nRet = 0;
    bool bRes = false;
   
    if (m_pVideoCodec != nullptr && m_pFormatCtx != nullptr)
    {
        // m_pVideoCodec = avcodec_find_decoder(m_pFormatCtx->streams[i]->codecpar->codec_id);
        m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
        nRet = avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar);

        if (m_pVideoCodecCtx != nullptr && m_pVideoCodecCtx->codec_id != 0 && nRet >= 0)
        {
            nRet = (avcodec_open2)(m_pVideoCodecCtx, m_pVideoCodec, nullptr);
            if (nRet != 0)
            {
                nRet = -1;
                std::cout << "Fail avcodec_open2" << std::endl;        
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

int FFmpegDecoder::DecodeVideo()
{
    int nRet = 0;
    int nFrameNumber  = 0;
    
    AVFrame *pFrameYUV = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    AVPacket *pPacket = av_packet_alloc();

    while (av_read_frame(m_pFormatCtx, pPacket) >= 0)
    {
        std::cout << "av_read_frame:" << pPacket->dts << " / pPacket /" << pPacket->pts << std::endl;
        
        if(pPacket->stream_index == m_nVideoStreamIndex)
        {
            nFrameNumber+=1;
            std::cout << "Read FrameNumber: " << nFrameNumber << std::endl;

            if (m_pVideoCodecCtx != nullptr)
            {
                nRet = avcodec_send_packet(m_pVideoCodecCtx, pPacket);

                std::cout << "avcodec_send_packet nRet: " << nRet << " " << pPacket->dts << " / pPacket /" << pPacket->pts << std::endl;

                
                if (nRet == 0)
                {
                    nRet = avcodec_receive_frame(m_pVideoCodecCtx, pFrameYUV);

                    if (nRet == 0)
                    {
                        nRet = ConvertRGBAframe(*pFrameYUV, pFrameRGB);
                        nRet = BMPSave(pFrameRGB, pFrameRGB->width, pFrameRGB->height);
                    }
                }
                else
                {
                    std::cout << "avcodec_send_packet - ErrorCode:" << nRet << std::endl;
                    nRet = -1;
                    return nRet;
                }
            }
        }
    }
    return nRet;
}

int FFmpegDecoder::ConvertRGBAframe(AVFrame pFrameYUV, AVFrame* pOutFrame)
{
    int nRet = 0;
    struct SwsContext *pSwsCtx = nullptr;
    int nInputWidth = pFrameYUV.width; 
    int nInputHeight = pFrameYUV.height;
    int nOutputWidth = pFrameYUV.width;
    int nOutputHeight = pFrameYUV.height; 

    std::cout << nInputWidth << std::endl;

    pSwsCtx = sws_getContext(nInputWidth, nInputHeight, AV_PIX_FMT_YUV420P,
                    nOutputWidth, nOutputHeight, AV_PIX_FMT_BGR24,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);

    pOutFrame->format = AV_PIX_FMT_BGR24;
    pOutFrame->width  = nInputWidth;
    pOutFrame->height = nInputHeight;

    uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, pOutFrame->width, pOutFrame->height, 1));
    av_image_fill_arrays(pOutFrame->data, pOutFrame->linesize, out_buffer, AV_PIX_FMT_RGB24, pOutFrame->width, pOutFrame->height, 1);

    sws_scale(pSwsCtx, (const uint8_t* const*)pFrameYUV.data, pFrameYUV.linesize, 0, 
                        nInputHeight, pOutFrame->data, pOutFrame->linesize);

    return nRet;
}

int FFmpegDecoder::BMPSave(AVFrame* pFrameRGB, int width, int height)
{

    std::cout << "start file" << std::endl;

    FILE* file;
    const char* filename = "FFmpeg_test.bmp";
    file = fopen(filename, "wb");
    int y;

    if (file) 
    {
        unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
        unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};
        long file_size = 54 + 3 * width * height;
        bmpfileheader[2] = (unsigned char)(file_size);
        bmpfileheader[3] = (unsigned char)(file_size >> 8);
        bmpfileheader[4] = (unsigned char)(file_size >> 16);
        bmpfileheader[5] = (unsigned char)(file_size >> 24);

        bmpinfoheader[4] = (unsigned char)(width);
        bmpinfoheader[5] = (unsigned char)(width >> 8);
        bmpinfoheader[6] = (unsigned char)(width >> 16);
        bmpinfoheader[7] = (unsigned char)(width >> 24);
        bmpinfoheader[8] = (unsigned char)(height);
        bmpinfoheader[9] = (unsigned char)(height >> 8);
        bmpinfoheader[10] = (unsigned char)(height >> 16);
        bmpinfoheader[11] = (unsigned char)(height >> 24);

        fwrite(bmpfileheader, 1, 14, file);
        fwrite(bmpinfoheader, 1, 40, file);

        for (y = height - 1; y >= 0; y--) {
            fwrite(pFrameRGB->data[0] + y * pFrameRGB->linesize[0], 3, width, file);
        }


        fclose(file);
    }
    else 
    {
        std::cerr << "Failed to open file for writing" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;

}


