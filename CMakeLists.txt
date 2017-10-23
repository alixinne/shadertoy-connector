cmake_minimum_required(VERSION 2.8)

project(shadertoy-connector)
include(FindPkgConfig)

# GL libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost 1.58 REQUIRED COMPONENTS filesystem log serialization
	date_time program_options)
find_package(CURL REQUIRED)
pkg_search_module(glfw3 REQUIRED glfw3)
pkg_search_module(jsoncpp REQUIRED jsoncpp)

# Directories
set(SOURCES_DIR "${CMAKE_SOURCE_DIR}/Sources")

# Set link directories
link_directories(${glfw3_LIBRARY_DIRS} ${jsoncpp_LIBRARY_DIRS})

# oglplus config
set(oglplus_CONFIGURE_EXTRA_ARGS --use-glew --use-glfw3)
set(shadertoy_LOAD_OGLPLUS ON)

# libshadertoy
find_package(shadertoy REQUIRED)

# shader settings
include_directories(
	${OPENGL_INCLUDE_DIRS}
	${GLEW_INCLUDE_DIRS}
	${glfw3_INCLUDE_DIRS}
	${oglplus_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${jsoncpp_INCLUDE_DIRS}
	${shadertoy_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/Sources)

# create the connector library
add_library(shadertoy_connector
	${SOURCES_DIR}/context.cpp
	${SOURCES_DIR}/host.cpp
	${SOURCES_DIR}/local.cpp
	${SOURCES_DIR}/remote.cpp)

# list of libraries to link into all the targets
set(COMMON_LIBRARIES
	${OPENGL_LIBRARY}
	${GLEW_LIBRARIES}
	${glfw3_LIBRARIES}
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${jsoncpp_LIBRARIES}
	${shadertoy_LIBRARIES}
	shadertoy_connector)

# Mathematica connector
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/FindMathematica/CMake/Mathematica")
find_package(Mathematica COMPONENTS MathLink)
if(Mathematica_FOUND)
	Mathematica_MathLink_MPREP_TARGET(
		${SOURCES_DIR}/shadertoy.tm
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shadertoy_mathematica.c)

	add_executable(shadertoy_mathematica
		${SOURCES_DIR}/shadertoy_mathematica.cpp
		${CMAKE_CURRENT_BINARY_DIR}/shadertoy_mathematica.c)

	if(WIN32)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	endif(WIN32)

	set_property(TARGET shadertoy_mathematica PROPERTY OUTPUT_NAME "shadertoy.bin")
	target_include_directories(shadertoy_mathematica PUBLIC ${Mathematica_MathLink_INCLUDE_DIR})
	target_link_libraries(shadertoy_mathematica
						  ${COMMON_LIBRARIES}
						  ${Mathematica_MathLink_LIBRARIES})

	add_custom_command(TARGET shadertoy_mathematica POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:shadertoy_mathematica>" "${CMAKE_SOURCE_DIR}/shadertoy.bin/${Mathematica_SYSTEM_ID}/$<TARGET_FILE_NAME:shadertoy_mathematica>")

	set(ST_MATH_INSTALL_DIR ${Mathematica_USERBASE_DIR}/Applications/Shadertoy)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/Kernel
			DESTINATION ${ST_MATH_INSTALL_DIR}
			COMPONENT st_mathematica)
	install(DIRECTORY ${CMAKE_SOURCE_DIR}/shadertoy.bin/${Mathematica_SYSTEM_ID}
			DESTINATION ${ST_MATH_INSTALL_DIR}/shadertoy.bin
			COMPONENT st_mathematica
			USE_SOURCE_PERMISSIONS)
	install(FILES ${CMAKE_SOURCE_DIR}/Shadertoy.m
			DESTINATION ${ST_MATH_INSTALL_DIR}
			COMPONENT st_mathematica)
endif()

# Octave connector
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Octave)
if(Octave_FOUND)
	octave_add_oct(shadertoy_octave
		SOURCES ${SOURCES_DIR}/shadertoy_octave.cpp
		LINK_LIBRARIES ${COMMON_LIBRARIES})

	install(TARGETS shadertoy_octave
			DESTINATION ${OCTAVE_OCT_FILE_DIR}
			COMPONENT st_octave)
endif()

# C++14 for the shadertoy_connector target. Others should be fine
set_property(TARGET shadertoy_connector PROPERTY CXX_STANDARD 14)

# Required
set_property(TARGET shadertoy_connector PROPERTY POSITION_INDEPENDENT_CODE ON)
