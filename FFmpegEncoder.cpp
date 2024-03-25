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

    m_pEncoderCodecCtx->bit_rate = 1132321;
    m_pEncoderCodecCtx->width = 1280;  
    m_pEncoderCodecCtx->height = 720;
    m_pEncoderCodecCtx->time_base = (AVRational){1, 25};
    m_pEncoderCodecCtx->framerate = (AVRational){25, 1};
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

int FFmpegEncoder::EncodeVideo(const AVFrame& pFrameData, std::ofstream& ofsH264File)
{
    int nRet = 0;
    AVPacket* pPacket = av_packet_alloc();


    while (true)
    {
        nRet = avcodec_send_frame(m_pEncoderCodecCtx, &pFrameData);

        if (nRet >= 0)
        {
            nRet = avcodec_receive_packet(m_pEncoderCodecCtx, pPacket);
            if (nRet == AVERROR(EAGAIN))
            {
                continue;
            }
             else if (nRet == AVERROR_EOF)
            {
                return -99;
            }

            if (nRet >= 0)
            {
                m_nEncoderCount++;
                std::cout << "m_nEncoderCount" << m_nEncoderCount << std::endl;
                ofsH264File.write((const char*)pPacket->data, pPacket->size);
                return nRet;
            }
            else
            {
                std::cout << "Fail Receive Encode Packet" << m_nEncoderCount << std::endl;
            }
        }
    }

    return nRet;
}
