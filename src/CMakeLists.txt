#
# Copyright (c) 2020 Kyrylo Zharenkov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
cmake_minimum_required(VERSION 3.0.4)
project(${CMAKE_PROJECT_NAME} VERSION ${CMAKE_PROJECT_VERSION})

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

find_package(Threads REQUIRED)

include_directories(
        SYSTEM
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

add_subdirectory(yaml-cpp)

function(collect_sources SOURCE_FOLDER SRC_LIST_OUT)
    set(SRC_LIST)
    file(GLOB_RECURSE SRC_LIST
            "${SOURCE_FOLDER}/*.cpp"
            "${SOURCE_FOLDER}/*.h")

    foreach(src_file ${SRC_LIST})
        if (src_file MATCHES  yaml-cpp)
            list(REMOVE_ITEM SRC_LIST ${src_file})
        endif()
    endforeach()

    set(${SRC_LIST_OUT} ${SRC_LIST} PARENT_SCOPE)
endfunction()

set(SRC_LIST)
collect_sources(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_include_directories(yaml-cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp/include)
target_compile_options(yaml-cpp PUBLIC -Wno-unused-parameter -Wno-shadow)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads yaml-cpp)
