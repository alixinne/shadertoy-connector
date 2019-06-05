add_executable(shadertoy_client
	${SRC_DIR}/cli/client.cpp)

target_link_libraries(shadertoy_client PUBLIC stc_client)

set_target_properties(shadertoy_client
	PROPERTIES CXX_STANDARD 17)

target_compile_options(shadertoy_client PUBLIC ${EXTRA_FLAGS})
