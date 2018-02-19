# shadertoy-connector installation

This toolkit is currently not available as binary packages. Please follow these
instructions to build from source.

## Building the package

The following Debian/Ubuntu packages are required for building the package:

* [libshadertoy-dev](https://gitlab.inria.fr/vtaverni/libshadertoy) >= 0.1.7
* cmake >= 3.1
* libgl1-mesa-dev
* libepoxy-dev >= 1.3
* libboost-all-dev >= 1.54 (or filesystem, log, serialization, date_time, program_options and system development packages)
* libglfw3-dev
* libcurl4-openssl-dev
* libjsoncpp-dev

The following additional packages are required for building the Mathematica
package:

* Mathematica (download from Wolfram site, the `math` program must be in `$PATH`)
* uuid-dev

The following additional packages are required for building the octave package:

* octave
* octave-pkg-dev

The following commands can be used to compile and install the project:

```bash
# Create a build directory and cd into it
$ mkdir build ; cd build

# Prepare the Makefiles
$ cmake ..

# Build
$ make -j8

# Install the Mathematica package
$ cmake -DCOMPONENT=st_mathematica -P cmake_install.cmake

# Install the Octave library
$ sudo cmake -DCOMPONENT=st_octave -P cmake_install.cmake
```
