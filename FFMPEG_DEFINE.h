#pragma once

// 
const int nMinCount = -0x11;

enum class FD_RESULT
    {
        WARNING_NO_VIEDO_STREAM = nMinCount,
        WARNING_NO_AUDIO_STREAM,
        ERROR_NO_AUDIO_AND_VIEDO_STREAM,
        OK,
    };