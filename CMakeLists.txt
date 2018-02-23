cmake_minimum_required(VERSION 3.1)

project(shadertoy-connector)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -Wall
set(CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

# GL libraries
find_package(OpenGL REQUIRED)
find_package(Boost 1.54 REQUIRED COMPONENTS filesystem log serialization
	date_time program_options system)
find_package(CURL REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(glfw3 REQUIRED glfw3)
link_directories(${glfw3_LIBDIR})

pkg_search_module(jsoncpp REQUIRED jsoncpp)
link_directories(${jsoncpp_LIBDIR})

pkg_search_module(EPOXY REQUIRED epoxy)
link_directories(${epoxy_LBIDIR})

# Directories
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# libshadertoy
find_package(shadertoy REQUIRED 0.1.7)

# Load omw
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/omw)

# setup shared include directories
include_directories(
	${INCLUDE_DIR}
	${jsoncpp_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${glfw3_INCLUDE_DIRS}
	${EPOXY_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS})

# create the connector library
add_library(shadertoy_connector OBJECT
	${SRC_DIR}/context.cpp
	${SRC_DIR}/host.cpp
	${SRC_DIR}/local.cpp
	${SRC_DIR}/remote.cpp)

# list of libraries to link into all the targets
set(COMMON_LIBRARIES
	${OPENGL_LIBRARY}
	${EPOXY_LIBRARIES}
	${glfw3_LIBRARIES}
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${jsoncpp_LIBRARIES}
	shadertoy)

# Mathematica connector
omw_add_mathematica(shadertoy_mathematica
	PACKAGE_NAME Shadertoy
	INSTALL st_mathematica
	MPREP_SOURCES ${SRC_DIR}/mathematica/Shadertoy.tm
	SOURCES ${SRC_DIR}/api.cpp $<TARGET_OBJECTS:shadertoy_connector>
	LINK_LIBRARIES ${COMMON_LIBRARIES})

# Octave connector
omw_add_octave(shadertoy_octave
	SOURCES ${SRC_DIR}/api.cpp $<TARGET_OBJECTS:shadertoy_connector>
	LINK_LIBRARIES ${COMMON_LIBRARIES}
	INSTALL st_octave)

# C++14 for the targets
set_property(TARGET shadertoy_connector PROPERTY CXX_STANDARD 14)

# Required
set_property(TARGET shadertoy_connector PROPERTY POSITION_INDEPENDENT_CODE ON)

# Test procedures
enable_testing()
file(GLOB TEST_FILES ${CMAKE_SOURCE_DIR}/t/*.t)
foreach(TEST_FILE ${TEST_FILES})
	get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)
	add_test(NAME ${TEST_FILE_NAME} COMMAND prove -v --color ${TEST_FILE} :: ${CMAKE_CURRENT_BINARY_DIR}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()
