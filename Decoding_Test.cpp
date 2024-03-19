#include "ffmpegDecode.h"


int main(int argc, char *argv[])
{
    const std::string input_file = "output.mp4";

    FFmpegDecoder decoder;
    decoder.OpenFile(input_file);
    decoder.DecodeVideo();

    return 0;
}
