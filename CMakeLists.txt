cmake_minimum_required(VERSION 3.8)
project(Audacious-Plugin-VGS)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        bgm_decoder/vgs2tone.c
        bgm_decoder/vgsdec.c
        bgm_decoder/vgsdec.h
        bgm_decoder/vgsmml.h
        bgm_decoder/vgsmml.c
        bgm_decoder/miniz.c
        VGSDecoder.cpp
        VGSDecoder.hpp)

add_library(vgsmml SHARED ${SOURCE_FILES})
target_link_libraries(vgsmml audcore)