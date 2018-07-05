# Find package for jsoncpp
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(jsoncpp_PKGCONF jsoncpp)

# Include directory
find_path(jsoncpp_INCLUDE_DIR
	NAMES json/json.h
	PATHS ${jsoncpp_PKGCONF_INCLUDE_DIRS})

# Library
find_library(jsoncpp_LIBRARY
	NAMES ${jsoncpp_PKGCONF_LIBARIES} jsoncpp
	PATHS ${jsoncpp_PKGCONF_LIBRARY_DIRS})

# Set include dir and libraries variables
set(jsoncpp_PROCESS_INCLUDES jsoncpp_INCLUDE_DIR)
set(jsoncpp_PROCESS_LIBS jsoncpp_LIBRARY)
libfind_process(jsoncpp)
