cmake_minimum_required(VERSION 3.22.1)

set(jniLibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(projectName moonmedia)

project(${projectName})

include_directories(
        include
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
        component/render
        component/render/audio
        component/render/video
        component/sync
)

link_directories(${jniLibs}/${ANDROID_ABI})

file(GLOB src-files
        ${CMAKE_SOURCE_DIR}/*cpp
        ${CMAKE_SOURCE_DIR}/egl/*cpp
        ${CMAKE_SOURCE_DIR}/egl/eglexample/*cpp
        ${CMAKE_SOURCE_DIR}/client/*cpp
        ${CMAKE_SOURCE_DIR}/client/recorder/*cpp
        ${CMAKE_SOURCE_DIR}/client/player/*cpp
        ${CMAKE_SOURCE_DIR}/component/base/*cpp
        ${CMAKE_SOURCE_DIR}/component/decoder/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/audio/*cpp
        ${CMAKE_SOURCE_DIR}/component/render/video/*cpp
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