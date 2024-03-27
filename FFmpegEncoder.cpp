#include "FFmpegEncoder.h"

FFmpegEncoder::FFmpegEncoder()
{
    m_pOutputFormatCtx = nullptr;
    m_pEncoderCodecCtx = nullptr;
    m_pEncoderCodec = nullptr;
    m_pVideoStream = nullptr;
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
    m_pEncoderCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    m_pEncoderCodecCtx = avcodec_alloc_context3(m_pEncoderCodec);

    m_pEncoderCodecCtx->bit_rate = 400000;
    m_pEncoderCodecCtx->width = 1280;  
    m_pEncoderCodecCtx->height = 720;
    m_pEncoderCodecCtx->time_base = (AVRational){1, 30};
    m_pEncoderCodecCtx->gop_size = 10;
    m_pEncoderCodecCtx->max_b_frames = 10;
    m_pEncoderCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;


    nRet = avcodec_open2(m_pEncoderCodecCtx, m_pEncoderCodec, nullptr);

    if (nRet < 0)
    {
        nRet = static_cast<int>(FD_RESULT::ERROR_FAIL_OPEN_CODEC);
        std::cout << "Fail avcodec_open2 Encoder" << std::endl;        
        return nRet;
    } 
    else
    {
        nRet = avformat_alloc_output_context2(&m_pOutputFormatCtx, nullptr, nullptr, ofsOutputFilePath.c_str());
        m_pVideoStream = avformat_new_stream(m_pOutputFormatCtx, m_pEncoderCodec);

        if (m_pVideoStream != nullptr && m_pEncoderCodecCtx != nullptr)
        {
            m_pVideoStream->time_base = m_pEncoderCodecCtx->time_base;
            nRet = avcodec_parameters_from_context(m_pVideoStream->codecpar, m_pEncoderCodecCtx);

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
        else if (m_pVideoStream == nullptr)
        {
            nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_ADD_STREAM);
            return nRet;   
        }
    }   

    return nRet;
}

int FFmpegEncoder::FlushEncodeVideo(const AVFrame& pFrameData)
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
            std::cout << "Encoded Frame: " << m_nEncoderCount << std::endl;
            return nRet;
        }
        else if (nRet >= 0)
        {
            // ofsOutputFile.write((const char*)pPacket->data, pPacket->size);
            nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pPacket);

            if (nRet < 0)
            {
                nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_WRITE_PACKET);
                std::cout << "Fail WRITE PACKET" << m_nEncoderCount << std::endl;
                return nRet;
            }
            m_nEncoderCount++;
            av_packet_unref(pPacket);
            continue;
        }
        else
            break;
    }
    return nRet;
}

int FFmpegEncoder::EncodeVideo(const AVFrame& pFrameData)
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
                return nRet;
            }
            if (nRet == AVERROR_EOF)
            {
                nRet = static_cast<int>(FD_RESULT::WARNING_ENCODER_END_FILE);
                return nRet;
            }

            if (nRet >= 0)
            {
                // ofsOutputFile.write((const char*)pPacket->data, pPacket->size);
                nRet = av_interleaved_write_frame(m_pOutputFormatCtx, pPacket);

                if (nRet < 0)
                {
                    nRet = static_cast<int>(FD_RESULT::ERROR_ENCODER_WRITE_PACKET);
                    std::cout << "Fail WRITE PACKET" << m_nEncoderCount << std::endl;
                    return nRet;
                }
                m_nEncoderCount++;

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
    avcodec_free_context(&m_pEncoderCodecCtx);
    avformat_free_context(m_pOutputFormatCtx);


    return nRet;
}
