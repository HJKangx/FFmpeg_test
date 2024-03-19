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

    m_nVideoStreamIndex = 0;
    m_nAudioStreamIndex = 0;
}

FFmpegDecoder::~FFmpegDecoder()
{
    std::cout << ("End FFmpegDecoder.") << std::endl;
}

int FFmpegDecoder::OpenFile(const std::string& strInputUrl)
{
    float fFps = 0.f;
    int nInputDuration = 0;
    int nTotalFrame = 0;

    std::cout << "OpenFile: " << strInputUrl << std::endl;

    if (avformat_open_input(&m_pFormatCtx, strInputUrl.c_str(), nullptr, nullptr) < 0)
	{
        std::cerr << "no avformat_open_input" << std::endl;
		return false;
	}

    if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
	{
        std::cerr << "no avformat_find_stream_info" << std::endl;
		return false;
	}

    av_dump_format(m_pFormatCtx, 0, 0, 0);

    for(int i = 0; i < m_pFormatCtx->nb_streams; i++)
    {
        if(m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            AVRational avrFps = m_pFormatCtx->streams[i]->avg_frame_rate;
            fFps = static_cast<float>(avrFps.num) / static_cast<float>(avrFps.den);
        }
    }

    nInputDuration = m_pFormatCtx->duration / AV_TIME_BASE;
    nTotalFrame = nInputDuration * fFps;

    std::cout << "Video Duration: " << nInputDuration << " seconds, " << fFps << "fps, " 
               << nTotalFrame << " frame" << std::endl;

    bool bCheckVideo = OpenVideo();

    return 0;
}

int FFmpegDecoder::OpenVideo()
{
    int nRet = 0;
    bool bRes = false;
    int nWidth;
    int nHeight;

    if (m_pFormatCtx)
    {
        m_nVideoStreamIndex = -1;

        for (int i = 0; i < m_pFormatCtx->nb_streams; i++)
        {
            m_nVideoStreamIndex = i;

            // pVideoCodecCtx = pFormatCtx->streams[i]->codec;
			m_pVideoCodec = avcodec_find_decoder(m_pFormatCtx->streams[i]->codecpar->codec_id);
            m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
            avcodec_parameters_to_context(m_pVideoCodecCtx, m_pFormatCtx->streams[i]->codecpar);

            if (m_pVideoCodec)
            {
                bRes = !(avcodec_open2)(m_pVideoCodecCtx, m_pVideoCodec, nullptr);
                nWidth = m_pVideoCodecCtx->width;
                nHeight = m_pVideoCodecCtx->height;
            }
            break;
        }
    }

    if (!bRes)
    {
        nRet = -1;
        std::cout << "no avcodec_open2" << std::endl;        
        return nRet;
    }
    else
    {
        pImgConvertCtx = sws_getContext(
            nWidth, nHeight,
            m_pVideoCodecCtx->pix_fmt,
            nWidth, nHeight,
            AV_PIX_FMT_BGR24, SWS_BICUBIC, nullptr, nullptr, nullptr);
    }

    return nRet;
}

int FFmpegDecoder::DecodeVideo()
{
    int nRet = 0;
    int nCnt  = 0;
    int isDecodeComplite = 0;
    

    if (m_nVideoStreamIndex != -1)
    {
        AVFrame *pFrameYUV = av_frame_alloc();
        AVPacket packet;


        while (av_read_frame(m_pFormatCtx, &packet) >= 0)
        {
            nCnt+=1;
            std::cout << "av_read_frame" << nCnt << std::endl;
            int64_t pts = 0;
            pts = (packet.dts != AV_NOPTS_VALUE) ? packet.dts : 0;

            if(packet.stream_index == m_nVideoStreamIndex)
            {

                if (m_pVideoCodecCtx)
                {
                    int got_picture_ptr = 0;
                    auto data1 = avcodec_send_packet(m_pVideoCodecCtx, &packet);
                    isDecodeComplite = avcodec_receive_frame(m_pVideoCodecCtx, pFrameYUV);
                   
                }

                if (!isDecodeComplite)
					{
                        struct SwsContext *sws_ctx = nullptr;
                        int srcW = pFrameYUV->width; 
                        int srcH = pFrameYUV->height;
                        int dstW = pFrameYUV->width;
                        int dstH = pFrameYUV->height; 

                        sws_ctx = sws_getContext(srcW, srcH, AV_PIX_FMT_YUV420P,
                         dstW, dstH, AV_PIX_FMT_BGR24,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);

                        AVFrame *frameRGB = av_frame_alloc();
                        frameRGB->format = AV_PIX_FMT_BGR24;
                        frameRGB->width  = pFrameYUV->width;
                        frameRGB->height = pFrameYUV->height;

                        uint8_t* out_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB24, frameRGB->width, frameRGB->height, 1));
                        av_image_fill_arrays(frameRGB->data, frameRGB->linesize, out_buffer, AV_PIX_FMT_RGB24, frameRGB->width, frameRGB->height, 1);

                        sws_scale(sws_ctx, (const uint8_t* const*)pFrameYUV->data, pFrameYUV->linesize,
                    0, pFrameYUV->height, frameRGB->data, frameRGB->linesize);



                        BMPSave(frameRGB, pFrameYUV->width, pFrameYUV->height);
					}
            }

        }

    }

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