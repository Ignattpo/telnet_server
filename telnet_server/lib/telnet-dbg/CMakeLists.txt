cmake_minimum_required(VERSION 3.5)


add_library(telnet_dbg SHARED
    telnet_dbg.c
    telnet_dbg.h
)

target_include_directories(telnet_dbg PRIVATE telnet-dbg)

set_target_properties(telnet_dbg PROPERTIES PUBLIC_HEADER telnet_dbg.h)
