add_library(stc_core STATIC
	${INCLUDE_DIR}/stc/core/basic_context.hpp
	${INCLUDE_DIR}/stc/core/basic_host.hpp
	${INCLUDE_DIR}/stc/core/getpid.h
	${INCLUDE_DIR}/stc/core/image.hpp

	${SRC_DIR}/core/basic_context.cpp
	${SRC_DIR}/core/basic_host.cpp
	${SRC_DIR}/core/image.cpp)

target_include_directories(stc_core PUBLIC
	${INCLUDE_DIR}
	${EPOXY_INCLUDE_DIRS})

target_link_libraries(stc_core PUBLIC ${Boost_LIBRARIES})

set_target_properties(stc_core
	PROPERTIES CXX_STANDARD 14
	POSITION_INDEPENDENT_CODE ON)

target_compile_options(stc_core PUBLIC ${EXTRA_ARGS})
