cmake_minimum_required(VERSION 3.1)

project(shadertoy-connector)

include(GNUInstallDirs)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GL libraries
find_package(OpenGL REQUIRED)
find_package(Boost 1.54 REQUIRED COMPONENTS filesystem date_time program_options system)
find_package(CURL REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(glfw3 REQUIRED glfw3)
link_directories(${glfw3_LIBDIR})

pkg_search_module(jsoncpp REQUIRED jsoncpp)
link_directories(${jsoncpp_LIBDIR})

pkg_search_module(EPOXY REQUIRED epoxy)
link_directories(${epoxy_LBIDIR})

if (WIN32)
	find_package(ZeroMQ)

	if (ZeroMQ_FOUND)
		set(ZMQ_LIBRARIES libzmq-static ws2_32 iphlpapi)
	endif()
else()
	pkg_search_module(ZMQ libzmq)
	link_directories(${ZMQ_LIBRARY_DIRS})
endif()

if (ZMQ_FOUND OR ZeroMQ_FOUND)
	set(HAS_ZMQ ON)
	set(ZMQ_OPT SHADERTOY_CONNECTOR_HAS_ZMQ=1)
	message(STATUS "Building with libzmq support")
else()
	set(HAS_ZMQ OFF)
	set(ZMQ_OPT SHADERTOY_CONNECTOR_HAS_ZMQ=0)
	message(STATUS "Building without libzmq support")
endif()

# Directories
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# libshadertoy
find_package(shadertoy 1.0.0 REQUIRED)

# Load omw
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/omw)

# Load components
add_subdirectory(${SRC_DIR}/core)
add_subdirectory(${SRC_DIR}/gl)

if (HAS_ZMQ)
	add_subdirectory(${SRC_DIR}/net)
	add_subdirectory(${SRC_DIR}/client)

	if (ST_BUILD_SERVER)
		add_subdirectory(${SRC_DIR}/server)
	endif()

	add_subdirectory(${SRC_DIR}/cli)

	# Extra libraries for connector clients
	set(EXTRA_LIBRARIES stc_client)
else()
	set(EXTRA_LIBRARIES "")
endif()

option(ST_BUILD_MATHEMATICA "Build the Mathematica target" ON)
option(ST_BUILD_OCTAVE "Build the Octave target" ON)

set(ST_SOURCES
	${INCLUDE_DIR}/stc/api.hpp
	${INCLUDE_DIR}/stc/host_manager.hpp

	${SRC_DIR}/api.cpp
	${SRC_DIR}/host_manager.cpp)

# Mathematica connector
if(ST_BUILD_MATHEMATICA)
	omw_add_mathematica(shadertoy_mathematica
		PACKAGE_NAME Shadertoy
		INSTALL st_mathematica
		MPREP_SOURCES ${SRC_DIR}/mathematica/Shadertoy.tm
		SOURCES ${ST_SOURCES}
		LINK_LIBRARIES stc_gl ${EXTRA_LIBRARIES}
		COMPILE_OPTIONS "-Wall"
		COMPILE_DEFINITIONS ${ZMQ_OPT})
endif()

# Octave connector
if(ST_BUILD_OCTAVE)
	omw_add_octave(shadertoy_octave
		SOURCES ${ST_SOURCES}
		LINK_LIBRARIES stc_gl ${EXTRA_LIBRARIES}
		INSTALL st_octave
		COMPILE_OPTIONS "-Wall"
		COMPILE_DEFINITIONS ${ZMQ_OPT})
endif()

# Test procedures
enable_testing()
file(GLOB TEST_FILES ${CMAKE_SOURCE_DIR}/t/*.t)
foreach(TEST_FILE ${TEST_FILES})
	get_filename_component(TEST_FILE_NAME ${TEST_FILE} NAME_WE)
	add_test(NAME ${TEST_FILE_NAME} COMMAND prove -v --color ${TEST_FILE} ::
		--path ${CMAKE_CURRENT_BINARY_DIR} --octave-pkg shadertoy_octave --mathematica-pkg Shadertoy
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()
