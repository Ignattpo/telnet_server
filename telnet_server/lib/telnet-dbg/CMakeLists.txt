cmake_minimum_required(VERSION 3.5)

find_package(Threads REQUIRED)

add_library(telnet_dbg SHARED
    telnet_dbg.c
    telnet_dbg.h
    telnet_dbg_priv.h
)

target_link_libraries(telnet_dbg
    PUBLIC  Threads::Threads)

target_include_directories(telnet_dbg PRIVATE telnet-dbg)

set_target_properties(telnet_dbg PROPERTIES PUBLIC_HEADER telnet_dbg.h)
