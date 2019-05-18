cmake_minimum_required(VERSION 2.8.3)
project(testarray)

add_definitions(-std=c++11)

find_package(Boost REQUIRED
        COMPONENTS
        python
        numpy
        QUIET)

find_package(PythonLibs 2.7 REQUIRED)
find_package(PythonInterp REQUIRED)

set (CMAKE_CXX_FLAGS "-O2")

include_directories(
        ${Boost_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
        ${PythonInterp_INCLUDE_DIRS}
)

add_library(testarray SHARED
        testarray.cpp
        )

set_target_properties(testarray PROPERTIES
        PREFIX ""
        )

target_link_libraries(testarray
        ${Boost_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${PythonInterp_LIBRARIES}
        )
