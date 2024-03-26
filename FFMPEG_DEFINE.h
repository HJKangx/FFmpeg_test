#pragma once

const int nMinCount = -0x10;

enum class FD_RESULT
{
    WARNING_FAIL_OPEN_INPUT = nMinCount,
    WARNING_FAIL_READ_PACKET,
    WARNING_NO_VIEDO_STREAM,
    WARNING_NO_AUDIO_STREAM,
    WARNING_ENCODER_END_FILE,
    WARNING_DECODER_END_FILE,
    ERROR_NO_AUDIO_AND_VIEDO_STREAM,
    ERROR_FAIL_OPEN_CODEC,
    ERROR_ENCODER_FAIL_RECEIVE_PACKET,
    ERROR_ENCODER_FAIL_SEND_FRAME,
    ERROR_ENCODER_FLUSH,
    ERROR_ENCODER_ADD_STREAM,
    ERROR_ENCODER_FILL_PARAMETER,
    ERROR_ENCODER_FAIL_OPEN_AVIO,
    ERROR_ENCODER_WRITE_PACKET,
    ERROR_ENCODER_WRITE_HEADER,
    OK,
};