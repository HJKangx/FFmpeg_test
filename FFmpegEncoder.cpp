#include "FFmpegEncoder.h"

FFmpegEncoder::FFmpegEncoder()
{
    m_pFormatCtx = nullptr;
    m_pEncoderCodecCtx = nullptr;
    // m_pVideoCodec = nullptr;
    m_pEncoderCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
}

FFmpegEncoder::~FFmpegEncoder()
{
    std::cout << "End FFmpegEncoder" << std::endl;
}




int FFmpegEncoder::EncodeVideo(AVFrame& pFrameYUV)
{
    int nRet = 0;
  
    return nRet;
}
