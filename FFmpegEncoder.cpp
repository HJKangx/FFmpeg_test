#include "FFmpegEncoder.h"

FFmpegEncoder::FFmpegEncoder()
{
    m_pFormatCtx = nullptr;
    m_pEncoderCodecCtx = nullptr;
    m_pEncoderCodec = nullptr;
    m_nEncoderCount = 0;
}

FFmpegEncoder::~FFmpegEncoder()
{
    std::cout << "End FFmpegEncoder" << std::endl;
}

int FFmpegEncoder::SetEncoder()
{
    int nRet = 0;
    m_pEncoderCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_pEncoderCodecCtx = avcodec_alloc_context3(m_pEncoderCodec);

    m_pEncoderCodecCtx->bit_rate = 1200000;
    m_pEncoderCodecCtx->width = 1280;  
    m_pEncoderCodecCtx->height = 720;
    m_pEncoderCodecCtx->time_base = (AVRational){1, 30};
    m_pEncoderCodecCtx->framerate = (AVRational){30, 1};
    m_pEncoderCodecCtx->gop_size = 10;
    m_pEncoderCodecCtx->max_b_frames = 1;
    m_pEncoderCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (m_pEncoderCodec->id == AV_CODEC_ID_H264)
    {
        av_opt_set(m_pEncoderCodecCtx->priv_data, "preset", "slow", 0);
    }
    nRet = avcodec_open2(m_pEncoderCodecCtx, m_pEncoderCodec, NULL);
    
    if (nRet < 0)
    {
        nRet = -1;
        std::cout << "Fail avcodec_open2 Encoder" << std::endl;        
        return nRet;
    } 

    return nRet;
}

int FFmpegEncoder::FlushEncodeVideo(const AVFrame& pFrameData, std::ofstream& ofsH264File)
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
            ofsH264File.write((const char*)pPacket->data, pPacket->size);
            av_packet_unref(pPacket);
            continue;
        }
        else
            break;
    }
    return nRet;
}

int FFmpegEncoder::EncodeVideo(const AVFrame& pFrameData, std::ofstream& ofsH264File)
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
                m_nEncoderCount++;
                pPacket->pts = av_rescale_q(m_nEncoderCount, (AVRational){1, 30}, m_pEncoderCodecCtx->time_base);
                // pPacket->pts = m_nEncoderCount * m_pEncoderCodecCtx->time_base.num / m_pEncoderCodecCtx->time_base.den;
                ofsH264File.write((const char*)pPacket->data, pPacket->size);
                // std::cout << "pPacket->pts: " << pPacket->pts << "pPacket->dts" << pPacket->dts<< std::endl;

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

    avformat_close_input(&m_pFormatCtx);
    avcodec_close(m_pEncoderCodecCtx);

    return nRet;
}
