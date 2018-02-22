cmake_minimum_required(VERSION 3.1)

project(shadertoy-connector)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# Set link directories
link_directories(${glfw3_LIBRARY_DIRS} ${jsoncpp_LIBRARY_DIRS})

# libshadertoy
find_package(shadertoy REQUIRED 0.1.7)

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
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/FindMathematica/CMake/Mathematica")
find_package(Mathematica COMPONENTS MathLink)
if(Mathematica_FOUND)
	set(ST_MATHEMATICA_MPREP_TARGET ${CMAKE_CURRENT_BINARY_DIR}/mathematica/st_tm.c)
	Mathematica_MathLink_MPREP_TARGET(${SRC_DIR}/mathematica/shadertoy.tm
		OUTPUT ${ST_MATHEMATICA_MPREP_TARGET})

	add_executable(shadertoy_mathematica
		${SRC_DIR}/shadertoy_mathematica.cpp
		${ST_MATHEMATICA_MPREP_TARGET}
		$<TARGET_OBJECTS:shadertoy_connector>)
	set_property(TARGET shadertoy_mathematica PROPERTY CXX_STANDARD 14)

	if(WIN32)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	endif(WIN32)

	# Set output name and configure kernel file
	set(ST_MATHEMATICA_OUTPUT_NAME "StConnectorMathematica")

	configure_file(${SRC_DIR}/mathematica/Shadertoy.m
				   ${CMAKE_CURRENT_BINARY_DIR}/mathematica/Shadertoy.m)
	set_property(TARGET shadertoy_mathematica PROPERTY OUTPUT_NAME ${ST_MATHEMATICA_OUTPUT_NAME})

	# Add Mathematica include directory and dependencies to the built program
	target_include_directories(shadertoy_mathematica PUBLIC ${Mathematica_MathLink_INCLUDE_DIR})
	target_link_libraries(shadertoy_mathematica
						  ${COMMON_LIBRARIES}
						  ${Mathematica_MathLink_LIBRARIES})

	# Install resulting files
	set(ST_MATH_INSTALL_DIR ${Mathematica_USERBASE_DIR}/Applications/Shadertoy)
	install(DIRECTORY ${SRC_DIR}/mathematica/Kernel
			DESTINATION ${ST_MATH_INSTALL_DIR}
			COMPONENT st_mathematica)
	install(TARGETS shadertoy_mathematica
			DESTINATION ${ST_MATH_INSTALL_DIR}/${Mathematica_SYSTEM_ID}
			COMPONENT st_mathematica)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mathematica/Shadertoy.m
			DESTINATION ${ST_MATH_INSTALL_DIR}
			COMPONENT st_mathematica)
endif()

# Octave connector
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Octave)
if(Octave_FOUND OR OCTAVE_EXECUTABLE)
	octave_add_oct(shadertoy_octave
		SOURCES ${SRC_DIR}/shadertoy_octave.cpp
		$<TARGET_OBJECTS:shadertoy_connector>
		LINK_LIBRARIES ${COMMON_LIBRARIES})
	set_property(TARGET shadertoy_octave PROPERTY CXX_STANDARD 14)

	install(TARGETS shadertoy_octave
			DESTINATION ${OCTAVE_OCT_FILE_DIR}
			COMPONENT st_octave)
endif()

# Test program for the connector library
add_executable(st_test
	${SRC_DIR}/shadertoy_test.cpp
	$<TARGET_OBJECTS:shadertoy_connector>)
target_link_libraries(st_test ${COMMON_LIBRARIES})
set_property(TARGET st_test PROPERTY CXX_STANDARD 14)

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
