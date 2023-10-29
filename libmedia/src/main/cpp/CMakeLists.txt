cmake_minimum_required(VERSION 3.22.1)

set(jniLibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(projectName moonmedia)

project(${projectName})

include_directories(
        ${jniLibs}/include
        glm
        utils
        component
        egl
        egl/eglexample
        client/player
        client/recorder
        component/base
        component/decoder
        component/decoder/audio
        component/decoder/video
        component/encoder
        component/encoder/audio
        component/encoder/video
        component/render
        component/render/audio
        component/render/video
        component/sync
)

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*cpp
        ${CMAKE_SOURCE_DIR}/egl/*cpp
        ${CMAKE_SOURCE_DIR}/egl/eglexample/*cpp
        ${CMAKE_SOURCE_DIR}/client/*cpp
        ${CMAKE_SOURCE_DIR}/client/recorder/*cpp
        ${CMAKE_SOURCE_DIR}/client/player/*cpp
        ${CMAKE_SOURCE_DIR}/component/base/*cpp
        ${CMAKE_SOURCE_DIR}/component/decoder/*cpp
        ${CMAKE_SOURCE_DIR}/component/decoder/audio/*cpp
        ${CMAKE_SOURCE_DIR}/component/decoder/video/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/audio/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/video/*cpp
        ${CMAKE_SOURCE_DIR}/component/encoder/*cpp
        ${CMAKE_SOURCE_DIR}/component/encoder/audio/*cpp
        ${CMAKE_SOURCE_DIR}/component/encoder/video/*cpp
        ${CMAKE_SOURCE_DIR}/component/sync/*cpp
        ${CMAKE_SOURCE_DIR}/nativeapi/*cpp
        ${CMAKE_SOURCE_DIR}/utils/*cpp
)

add_library( # Sets the name of the library.
        ${projectName}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
)

add_library(x264 SHARED IMPORTED)
set_target_properties(x264
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libx264.so)

add_library(mp3lame SHARED IMPORTED)
set_target_properties(mp3lame
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libmp3lame.so)

add_library(fdk-aac SHARED IMPORTED)
set_target_properties(fdk-aac
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libfdk-aac.so)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${jniLibs}/${ANDROID_ABI}/libswscale.so)

set(third-party-libs
        x264
        fdk-aac
        mp3lame
        avformat
        avcodec
        avfilter
        swresample
        swscale
        avutil)

set(native-libs
        android
        mediandk
        OpenSLES
        EGL
        GLESv3
        log)

target_link_libraries( # Specifies the target library.
        ${projectName}

        # Links the target library to the log library
        # included in the NDK.
        ${third-party-libs}
        ${native-libs}
)