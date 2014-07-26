cmake_minimum_required (VERSION 2.6)

# possible values: Release, Debug, RelWithDebugInfo
# set(CMAKE_BUILD_TYPE RelWithDebugInfo)

project(TEQ)
set(PROJECT_VERSION "0.0.1")

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_definitions(-DPROJECT_VERSION=\"${PROJECT_VERSION}\")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wconversion -Werror -pthread -std=c++11 -fPIC -fmax-errors=1")

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -march=native -msse -mfpmath=sse")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -msse -mfpmath=sse")
	add_definitions("-DNDEBUG")
endif()

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


#
# BUILD DEPENDENCIES:
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(JACK REQUIRED jack)

find_package(PythonLibs REQUIRED)

unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS python REQUIRED)

#
# BUILD TARGETS
#

include_directories(${PROJECT_SOURCE_DIR})

include_directories(${JACK_INCLUDE_DIRS})

# The main library
add_library(teq SHARED teq/teq.cc)
target_link_libraries(teq ${JACK_LIBRARIES})

# Python bindings
include_directories(${PYTHON_INCLUDE_DIRS})

message(STATUS "Adding target for Python module...")

add_library(pyteq SHARED teq/python.cc)
set_target_properties(pyteq PROPERTIES PREFIX "")
set_target_properties(pyteq PROPERTIES OUTPUT_NAME "teq")
target_link_libraries(pyteq ${JACK_LIBRARIES}  ${PYTHON_LIBRARIES} ${Boost_LIBRARIES} teq)
