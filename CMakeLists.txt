cmake_minimum_required (VERSION 2.6)
project (teenypath)

set(PROJECT_ROOT_PATH ${CMAKE_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_PATH}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_ROOT_PATH}/lib)
include_directories(${PROJECT_ROOT_PATH}/include)

if(UNIX)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++1y -fPIC -Wall -Werror")
else()
    add_definitions(-D_UNICODE)
endif()

set(SOURCE
    include/teenypath.h
    src/teenypath.cpp
)

set(TESTSOURCE
    ${SOURCE}
    test/teenypathtest.cpp
)

add_library(teenypath ${SOURCE})
add_executable(test_teenypath ${TESTSOURCE})
