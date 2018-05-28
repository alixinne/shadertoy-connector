add_executable(shadertoy_client
	${SRC_DIR}/cli/client.cpp)

target_link_libraries(shadertoy_client PUBLIC stc_client)

target_include_directories(shadertoy_client PRIVATE
	${PROJECT_SOURCE_DIR}/3rdparty/spdlog/include)

set_target_properties(shadertoy_client
	PROPERTIES CXX_STANDARD 14)

target_compile_options(shadertoy_client PUBLIC ${EXTRA_FLAGS})
