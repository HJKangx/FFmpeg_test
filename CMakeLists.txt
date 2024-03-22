cmake_minimum_required(VERSION 3.17)
set(TEST_VERSION "1.00.00")
project(FFmpegTest VERSION ${TEST_VERSION} LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
find_package(fmt REQUIRED)

pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libswscale
    libavutil
    )

set(FFMPEG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ThirdParty/ffmpeg)

add_executable(${PROJECT_NAME} 
    FFmpegDecoder.cpp
    FFmpegEncoder.cpp
    FFmpegTest.cpp
    )

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${FFMPEG_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    )

target_link_libraries(${PROJECT_NAME}
    PkgConfig::LIBAV
    fmt::fmt
    )