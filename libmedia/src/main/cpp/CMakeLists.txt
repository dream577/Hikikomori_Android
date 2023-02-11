cmake_minimum_required(VERSION 3.22.1)

project("moonmedia")
set(EXTERNAL_LIB_DIR ${CMAKE_SOURCE_DIR}/libs)

include_directories(
        include
        glm
        media
        media/common
        media/decoder
        media/decoder/audio
        media/decoder/video
        media/render
        media/render/audio
        media/render/video
        media/input
        media/output
        media/queue
        media/sync
        media/util
)

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*cpp
        ${CMAKE_SOURCE_DIR}/NativePlayer.cpp
        ${CMAKE_SOURCE_DIR}/media/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/audio/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/video/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/audio/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/video/*cpp
        ${CMAKE_SOURCE_DIR}/media/input/*cpp
        ${CMAKE_SOURCE_DIR}/media/output/*cpp
        ${CMAKE_SOURCE_DIR}/media/queue/*cpp
        ${CMAKE_SOURCE_DIR}/media/sync/*cpp
        )

add_library( # Sets the name of the library.
        moonmedia

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
        )

add_library(mp3lame SHARED IMPORTED)
set_target_properties(mp3lame
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libmp3lame.so)

add_library(x264 SHARED IMPORTED)
set_target_properties(x264
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libx264.x.so)

add_library(fdk-aac SHARED IMPORTED)
set_target_properties(fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libfdk-aac.so)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${EXTERNAL_LIB_DIR}/${ANDROID_ABI}/libswscale.so)


target_link_libraries( # Specifies the target library.
        moonmedia

        mp3lame
        x264
        fdk-aac
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale

        # Links the target library to the log library
        # included in the NDK.
        android
        mediandk
        OpenSLES
        EGL
        GLESv3
        log
        )