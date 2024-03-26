#include "FFmpegEncoder.h"

FFmpegEncoder::FFmpegEncoder()
{
    m_pOutputFormatCtx = nullptr;
    m_pOutputFormat = nullptr;
    m_pEncoderCodecCtx = nullptr;
    m_pEncoderCodec = nullptr;
    videoStream = nullptr;
    m_nEncoderCount = 0;
}

FFmpegEncoder::~FFmpegEncoder()
{
    CloseEncoder();
    std::cout << "End FFmpegEncoder" << std::endl;
}

int FFmpegEncoder::SetEncoder(const std::string& ofsOutputFilePath)
{
    int nRet = 0;
    m_pOutputFormat = av_guess_format(NULL, ofsOutputFilePath.c_str(), NULL);

    m_pEncoderCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_pEncoderCodecCtx = avcodec_alloc_context3(m_pEncoderCodec);

    // m_pEncoderCodecCtx->bit_rate = 400000;
    m_pEncoderCodecCtx->bit_rate = 1200000;
    m_pEncoderCodecCtx->width = 1280;  
    m_pEncoderCodecCtx->height = 720;
    m_pEncoderCodecCtx->time_base = (AVRational){1, 30};
    m_pEncoderCodecCtx->gop_size = 10;
    m_pEncoderCodecCtx->max_b_frames = 1;
    m_pEncoderCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    nRet = avcodec_open2(m_pEncoderCodecCtx, m_pEncoderCodec, nullptr);
    
    if (nRet < 0)
    {
        nRet = -1;
        std::cout << "Fail avcodec_open2 Encoder" << std::endl;        
        return nRet;
    } 
    else
    {
        // nRet = avformat_alloc_output_context2(&m_pOutputFormatCtx, nullptr, nullptr, ofsOutputFilePath.c_str());
        m_pOutputFormatCtx = avformat_alloc_context();
        m_pOutputFormatCtx->oformat = m_pOutputFormat;        
        videoStream = avformat_new_stream(m_pOutputFormatCtx, m_pEncoderCodec);


		// uint8_t* pVideoEncodeBuffer = (uint8_t *)av_malloc(10000000);

        if (videoStream != nullptr && m_pEncoderCodecCtx != nullptr)
        {

            videoStream->codecpar->codec_id = m_pEncoderCodec->id;
            videoStream->codecpar->codec_tag = 0;
            videoStream->codecpar->width = m_pEncoderCodecCtx->width;
            videoStream->codecpar->height = m_pEncoderCodecCtx->height;
            videoStream->codecpar->format = m_pEncoderCodecCtx->pix_fmt;
            videoStream->time_base = m_pEncoderCodecCtx->time_base;
            
            nRet = avcodec_parameters_from_context(videoStream->codecpar, m_pEncoderCodecCtx);

            if (nRet >= 0)
            {
                nRet = avio_open(&m_pOutputFormatCtx->pb, ofsOutputFilePath.c_str(), AVIO_FLAG_WRITE);
                if (nRet >= 0)
                {
                    nRet = avformat_write_header(m_pOutputFormatCtx, nullptr);
                    if (nRet < 0)
                    {
                        std::cout << "ERROR_ENCODER_WRITE_HEADER" <<std::endl;
                        nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_WRITE_HEADER);
                        return nRet; 
                    }
                }
                else
                {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_FAIL_OPEN_AVIO);
                return nRet; 
                }
            }
            else
            {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_FILL_PARAMETER);
                return nRet; 
            }
        }
        else if (videoStream == nullptr)
        {
            nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_ADD_STREAM);
            return nRet;   
        }
    }   
    return nRet;
}

int FFmpegEncoder::FlushEncodeVideo(const AVFrame& pFrameData, std::ofstream& ofsOutputFile)
{
    int nRet = 0;
    AVPacket* pPacket = av_packet_alloc();

    nRet = avcodec_send_frame(m_pEncoderCodecCtx, nullptr);

    while (true)
    {
        nRet = avcodec_receive_packet(m_pEncoderCodecCtx, pPacket);

        if (nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
        {
            nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_FLUSH);
            return nRet;
        }
        else if (nRet >= 0)
        {
            pPacket->pts = pFrameData.pts;
            ofsOutputFile.write((const char*)pPacket->data, pPacket->size);
            // nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pPacket);
            nRet = av_write_frame(m_pOutputFormatCtx, pPacket);

            if (nRet < 0)
            {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_WRITE_PACKET);
                std::cout << "Fail WRITE PACKET" << m_nEncoderCount << std::endl;
                return nRet;
            }
            m_nEncoderCount++;
            std::cout << " WRITE" << m_nEncoderCount << std::endl;
            av_packet_unref(pPacket);
            continue;
        }
        else
            break;
    }
    return nRet;
}

int FFmpegEncoder::EncodeVideo(const AVFrame& pFrameData, std::ofstream& ofsOutputFile)
{
    int nRet = 0;
    AVPacket* pPacket = av_packet_alloc();

    nRet = avcodec_send_frame(m_pEncoderCodecCtx, &pFrameData);

    while (true)
    {
        if (nRet >= 0)
        {
            nRet = avcodec_receive_packet(m_pEncoderCodecCtx, pPacket);
            if (nRet == AVERROR(EAGAIN))
            {
                continue;
            }
            if (nRet == AVERROR_EOF)
            {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_END_FILE);
                return nRet;
            }

            if (nRet >= 0)
            {
                pPacket->pts = pFrameData.pts;
                ofsOutputFile.write((const char*)pPacket->data, pPacket->size);
                // nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pPacket);
                nRet = av_write_frame(m_pOutputFormatCtx, pPacket);


                if (nRet < 0)
                {
                    nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_WRITE_PACKET);
                    std::cout << "Fail WRITE PACKET" << m_nEncoderCount << std::endl;
                    return nRet;
                }
                m_nEncoderCount++;
                std::cout << " WRITE" << m_nEncoderCount << std::endl;

                av_packet_unref(pPacket);
                return nRet;
            }
            else
            {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_FAIL_RECEIVE_PACKET);
                std::cout << "Fail Receive Encode Packet" << m_nEncoderCount << std::endl;
                return nRet;
            }
        }
        else
        {
            nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_FAIL_SEND_FRAME);
            std::cout << "Fail Receive Frame Packet" << m_nEncoderCount << std::endl;
            return nRet;
        }
    }

    return nRet;
}

int FFmpegEncoder::CloseEncoder()
{
    int nRet = 0;

    av_write_trailer(m_pOutputFormatCtx);
    avio_close(m_pOutputFormatCtx->pb);
    avformat_free_context(m_pOutputFormatCtx);

    avcodec_close(m_pEncoderCodecCtx);

    return nRet;
}
