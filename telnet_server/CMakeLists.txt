cmake_minimum_required(VERSION 3.9)

project(telnet-dbg VERSION 1.0.0 DESCRIPTION "lib telnet-dbg")

find_package(Threads REQUIRED)

include(GNUInstallDirs)

add_library(telnet-dbg SHARED
    telnet_dbg.c
    telnet_dbg_connections.c
    telnet_dbg_comm.c
    telnet_dbg.h
    telnet_dbg_priv.h
    telnet_dbg_comm.h
)

target_link_libraries(telnet-dbg
    PUBLIC  Threads::Threads
    PUBLIC  ${CMAKE_DL_LIBS})

add_compile_definitions(_GNU_SOURCE)

set_target_properties(telnet-dbg PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER telnet_dbg.h)

configure_file(telnet-dbg.pc.in telnet-dbg.pc @ONLY)

target_include_directories(telnet-dbg PRIVATE .)

install(TARGETS telnet-dbg
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/telnet-dbg.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
