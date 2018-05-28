add_executable(shadertoy_server
	${INCLUDE_DIR}/stc/server/host_server.hpp

	${SRC_DIR}/server/host_server.cpp
	${SRC_DIR}/server/main.cpp)

target_link_libraries(shadertoy_server PUBLIC stc_net stc_gl)

target_include_directories(shadertoy_server PRIVATE
	${PROJECT_SOURCE_DIR}/3rdparty/spdlog/include)

set_target_properties(shadertoy_server
	PROPERTIES CXX_STANDARD 14)

target_compile_options(shadertoy_server PUBLIC "-Wall")

install(TARGETS shadertoy_server
	RUNTIME DESTINATION bin
	COMPONENT st_server)
