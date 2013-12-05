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

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread -std=c++0x -fPIC")

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -march=native -msse -mfpmath=sse")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -msse -mfpmath=sse")
	add_definitions("-DNDEBUG")
endif()

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


#
# BUILD DEPENDENCIES:
#

message(STATUS "Checking if pkg-config is available...")
find_package(PkgConfig REQUIRED)

pkg_check_modules(JACK REQUIRED jack)

message(STATUS "checking for Python C library")	
find_package(PythonLibs)
if (PYTHONLIBS_FOUND)
	message(STATUS "Python C library found. Let's see if there's boost python, too...")

	message(STATUS "Checking Python3 support first...")
	find_package(Boost COMPONENTS python3)
	
	if(Boost_FOUND)
		message(STATUS "Found!")
	else()
		message(STATUS "Checking Python2 support...")
		find_package(Boost COMPONENTS python)
	endif()

	set(Boost_USE_STATIC_LIBS OFF)
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME OFF)
	
else()
	message(STATUS "Python C library not found. Not building Python module...")
endif()

#
# BUILD TARGETS
#

include_directories(${PROJECT_SOURCE_DIR})

include_directories(${JACK_INCLUDE_DIRS})

# The main library
add_library(teq SHARED teq/teq.cc)
target_link_libraries(teq ${JACK_LIBRARIES})

# Python bindings
if (PYTHONLIBS_FOUND)
	if (PYTHONLIBS_FOUND)
		include_directories(${PYTHON_INCLUDE_DIRS})
	endif()
	message(STATUS "Adding target for Python module...")
	add_library(pyteq SHARED teq/python.cc)
	set_target_properties(pyteq PROPERTIES PREFIX "")
	set_target_properties(pyteq PROPERTIES OUTPUT_NAME "teq")
	target_link_libraries(pyteq ${JACK_LIBRARIES} teq ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
endif()