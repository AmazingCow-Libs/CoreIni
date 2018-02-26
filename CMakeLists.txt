##~---------------------------------------------------------------------------##
##                     _______  _______  _______  _     _                     ##
##                    |   _   ||       ||       || | _ | |                    ##
##                    |  |_|  ||       ||   _   || || || |                    ##
##                    |       ||       ||  | |  ||       |                    ##
##                    |       ||      _||  |_|  ||       |                    ##
##                    |   _   ||     |_ |       ||   _   |                    ##
##                    |__| |__||_______||_______||__| |__|                    ##
##                             www.amazingcow.com                             ##
##  File      : CMakeLists.txt                                                ##
##  Project   : CoreIni                                                       ##
##  Date      : Feb 25, 2018                                                  ##
##  License   : GPLv3                                                         ##
##  Author    : n2omatt <n2omatt@amazingcow.com>                              ##
##  Copyright : AmazingCow - 2018                                             ##
##                                                                            ##
##  Description :                                                             ##
##                                                                            ##
##---------------------------------------------------------------------------~##

cmake_minimum_required(VERSION 3.5)

##------------------------------------------------------------------------------
## Project Settings.
project(CoreIni)


##------------------------------------------------------------------------------
## Sources.
add_library(CoreIni
    CoreIni/src/Ini.cpp
)


##------------------------------------------------------------------------------
## Include directories.
target_include_directories(CoreIni PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


##------------------------------------------------------------------------------
## Dependencies.
target_link_libraries(CoreIni LINK_PUBLIC CoreAssert)
target_link_libraries(CoreIni LINK_PUBLIC CoreFS    )
target_link_libraries(CoreIni LINK_PUBLIC CoreFile  )
target_link_libraries(CoreIni LINK_PUBLIC CoreString)
