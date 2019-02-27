# shadertoy-connector installation

This toolkit is currently not available as binary packages. Please follow these
instructions to build from source.

<!-- MDTOC maxdepth:6 firsth1:0 numbering:0 flatten:0 bullets:1 updateOnSave:1 -->

- [Getting the source](#Getting-the-source)   
- [Building the package](#Building-the-package)   
   - [Building the Mathematica package](#Building-the-Mathematica-package)   
   - [Building the Octave package](#Building-the-Octave-package)   
- [Testing the package](#Testing-the-package)   

<!-- /MDTOC -->

## Getting the source

To fetch the source code for this package, use the following command:

```bash
# git needs to be installed
$ git clone --recursive https://gitlab.inria.fr/vtaverni/shadertoy-connector.git

$ cd shadertoy-connector
```

## Building the package

The following Debian/Ubuntu packages are required for building the package:

* [libshadertoy-dev](https://gitlab.inria.fr/vtaverni/libshadertoy) = 1.0.0~rc6
* cmake >= 3.1
* libgl1-mesa-dev
* libepoxy-dev >= 1.3
* libboost-all-dev >= 1.54 (or filesystem, log, serialization, date_time, program_options and system development packages)
* libglfw3-dev
* libcurl4-openssl-dev
* libjsoncpp-dev

### Building the Mathematica package

The following additional packages are required for building the Mathematica
package:

* Mathematica (download from Wolfram site, the `math` program must be in `$PATH`)
* uuid-dev

The following commands can be used to compile and install the project:

```bash
# Create a build directory and cd into it
$ mkdir build ; cd build

# Prepare the Makefiles
$ cmake ..

# Build
$ make -j8

# Install the Mathematica package in the system directory
$ sudo cmake -DCOMPONENT=st_mathematica -P cmake_install.cmake
```

### Building the Octave package

The following additional packages are required for building the octave package:

* octave
* liboctave-dev

The following commands can be used to compile and install the project:

```bash
# Create a build directory and cd into it
$ mkdir build ; cd build

# Prepare the Makefiles
$ cmake ..

# Build
$ make -j8

# Install the Octave library in the system directory
$ sudo cmake -DCOMPONENT=st_octave -P cmake_install.cmake
```

## Testing the package

This package includes automated tests to check the built library is functioning
correctly. In the previously created `build` directory, these tests can be run
using the following command.

```bash
# ctest comes with cmake
$ ctest --verbose
```

The individual test files are located in the `t/` directory of this repository.
