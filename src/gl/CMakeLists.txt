add_library(stc_gl STATIC
	${INCLUDE_DIR}/stc/gl/context.hpp
	${INCLUDE_DIR}/stc/gl/host.hpp
	${INCLUDE_DIR}/stc/gl/local.hpp
	${INCLUDE_DIR}/stc/gl/remote.hpp

	${SRC_DIR}/gl/context.cpp
	${SRC_DIR}/gl/host.cpp
	${SRC_DIR}/gl/local.cpp
	${SRC_DIR}/gl/remote.cpp)

target_link_libraries(stc_gl PUBLIC stc_core
	${EPOXY_LIBRARIES}
	${GLFW3_LIBRARIES}
	${CURL_LIBRARIES}
	${jsoncpp_LIBRARIES}
	shadertoy-shared)

target_include_directories(stc_gl PRIVATE
	${jsoncpp_INCLUDE_DIRS})

set_target_properties(stc_gl
	PROPERTIES CXX_STANDARD 14
	POSITION_INDEPENDENT_CODE ON)

target_compile_options(stc_gl PUBLIC ${EXTRA_ARGS})
