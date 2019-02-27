add_library(stc_net STATIC
	${INCLUDE_DIR}/stc/net/io.hpp
	${SRC_DIR}/net/io.cpp)

target_link_libraries(stc_net PUBLIC
	stc_core
	${ZMQ_LIBRARIES})

if(NOT WIN32)
	target_link_libraries(stc_net PUBLIC pthread)
endif()

set_target_properties(stc_net
	PROPERTIES CXX_STANDARD 14
	POSITION_INDEPENDENT_CODE ON)

target_compile_options(stc_net PUBLIC ${EXTRA_ARGS})
