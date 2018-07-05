
# Find package for glfw3
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(glfw3_PKGCONF glfw3)

# Use find_package
find_package(glfw3 CONFIG QUIET)

# Include directory
find_path(GLFW3_INCLUDE_DIR_FIND
	NAMES GLFW/glfw3.h
	PATHS ${glfw3_PKGCONF_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})

# Library
find_library(GLFW3_LIBRARY_FIND
	NAMES ${glfw3_PKGCONF_LIBRARIES} glfw3 glfw3dll
	PATHS ${glfw3_PKGCONF_LIBRARY_DIRS} ${glfw3_LIBRARY_DIRS})

# Set include dir and libraries variables
set(GLFW3_PROCESS_INCLUDES GLFW3_INCLUDE_DIR_FIND)
set(GLFW3_PROCESS_LIBS GLFW3_LIBRARY_FIND)
libfind_process(GLFW3)
