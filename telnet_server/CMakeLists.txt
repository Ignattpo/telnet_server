cmake_minimum_required(VERSION 3.5)

project(telnet_server LANGUAGES C)

add_subdirectory(lib/)

add_executable(telnet_server main.c)

include_directories(lib/)
target_link_libraries(telnet_server
    PRIVATE telnet_dbg)

