cmake_minimum_required(VERSION 3.4.1)


# cmakelists 设置c++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")

endif()
# cmakelists 设置c++11
#file(GLOB native_srcs "src/main/cpp/*.cpp")

set(DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(${DIR})

add_library(
        native-lib
        SHARED
        native-lib.cpp
        MAudio.cpp
        MCallJava.cpp
        MFFmpeg.cpp
        MQueue.cpp
        MPlaystatus.cpp
        SoundTouch/AAFilter.cpp
        SoundTouch/SoundTouch.cpp
        SoundTouch/FIFOSampleBuffer.cpp
        SoundTouch/InterpolateCubic.cpp
        SoundTouch/InterpolateLinear.cpp
        SoundTouch/InterpolateShannon.cpp
        SoundTouch/BPMDetect.cpp
        SoundTouch/TDStretch.cpp
        SoundTouch/PeakFinder.cpp
        SoundTouch/RateTransposer.cpp
        SoundTouch/cpu_detect_x86.cpp
        SoundTouch/mmx_optimized.cpp
        SoundTouch/sse_optimized.cpp
        SoundTouch/FIRFilter.cpp

)

find_library(
        log-lib
        log)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libswscale.so)


target_link_libraries(
        native-lib
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        OpenSLES
        ${log-lib})