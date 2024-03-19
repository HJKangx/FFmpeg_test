cmake_minimum_required(VERSION 3.17)
set(TEST_VERSION "1.00.00")
project(FFmpegTest VERSION ${TEST_VERSION} LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)


set(FFMPEG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ThirdParty/ffmpeg)
set(LIBACODEC_LIBRARIES ${CMAKE_SOURCE_DIR}/ThirdParty/ffmpeg/libavcodec/)

add_executable(${PROJECT_NAME} 
    ffmpegDecode.cpp
    Decoding_Test.cpp)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/")

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${FFMPEG_INCLUDE_DIRS}
    ${LIBACODEC_LIBRARIES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    )


target_link_libraries(${PROJECT_NAME}
    PkgConfig::LIBAV
)