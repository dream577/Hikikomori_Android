cmake_minimum_required(VERSION 3.22.1)

set(jniLibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(projectName moonmedia)

project(${projectName})

include_directories(
        include
        glm
        utils
        media
        media/egl
        media/eglexample
        media/common
        media/encoder
        media/encoder/audio
        media/encoder/video
        media/decoder
        media/decoder/audio
        media/decoder/video
        media/recorder
        media/render
        media/render/audio
        media/render/video
        media/input
        media/output
        media/queue
        media/sync
        media/util
)

link_directories(${jniLibs}/${ANDROID_ABI})

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*cpp
        ${CMAKE_SOURCE_DIR}/utils/*cpp
        ${CMAKE_SOURCE_DIR}/media/*cpp
        ${CMAKE_SOURCE_DIR}/media/common/*cpp
        ${CMAKE_SOURCE_DIR}/media/egl/*cpp
        ${CMAKE_SOURCE_DIR}/media/eglexample/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/audio/*cpp
        ${CMAKE_SOURCE_DIR}/media/decoder/video/*cpp
        ${CMAKE_SOURCE_DIR}/media/encoder/*cpp
        ${CMAKE_SOURCE_DIR}/media/encoder/audio/*cpp
        ${CMAKE_SOURCE_DIR}/media/encoder/video/*cpp
        ${CMAKE_SOURCE_DIR}/media/recorder/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/audio/*cpp
        ${CMAKE_SOURCE_DIR}/media/render/video/*cpp
        ${CMAKE_SOURCE_DIR}/media/input/*cpp
        ${CMAKE_SOURCE_DIR}/media/output/*cpp
        ${CMAKE_SOURCE_DIR}/media/queue/*cpp
        ${CMAKE_SOURCE_DIR}/media/sync/*cpp
        ${CMAKE_SOURCE_DIR}/media/util/*cpp
        )

add_library( # Sets the name of the library.
        ${projectName}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        ${src-files}
        )

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
        ${log-lib}
        ${third-party-libs}
        ${native-libs}
        )