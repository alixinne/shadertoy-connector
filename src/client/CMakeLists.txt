add_library(stc_client STATIC
	${INCLUDE_DIR}/stc/client/net_host.hpp
	${SRC_DIR}/client/net_host.cpp)

target_link_libraries(stc_client PUBLIC stc_net)

target_include_directories(stc_client PRIVATE
	${PROJECT_SOURCE_DIR}/3rdparty/spdlog/include)

set_target_properties(stc_client
	PROPERTIES CXX_STANDARD 14
	POSITION_INDEPENDENT_CODE ON)

target_compile_options(stc_client PUBLIC "-Wall")
