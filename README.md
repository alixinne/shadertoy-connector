# Shadertoy Connector Package

This package is based on [libshadertoy](https://gitlab.inria.fr/vtaverni/libshadertoy)
to provide support for rendering Shadertoys as images directly from Mathematica
and Octave.

## Building the package

The following packages are required for building the package:

* libshadertoy-dev
* cmake
* libglew-dev
* libboost-all-dev
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

The following commands can be used to compile the project:

```bash
# Create a build directory and cd into it
$ mkdir build ; cd build

# Prepare the Makefiles
$ cmake ..

# Build
$ make -j

# Install the Mathematica package
$ cmake -DCOMPONENT=st_mathematica -P cmake_install.cmake

# Install the Octave library
$ sudo cmake -DCOMPONENT=st_octave -P cmake_install.cmake
```

## Usage

### Mathematica

The default install target mentioned above puts this package in the user package
load path, so it can be loaded directly from any session.

```mathematica
(* Load the Shadertoy package *)
<< Shadertoy`

(* Render a specific frame of a Shadertoy, with given inputs *)
Manipulate[
	RenderShadertoy["llySRh", Frame -> n, Mouse -> {x, y}],
		{n, 0, 10, 1}, {x, 0, 640}, {y, 0, 360}];

(* Compile some GLSL source and render it *)
shaderId = CompileShadertoy["void mainImage(out vec4 O, in vec2 U){O = vec4(sin(iTime*.1), cos(iTime*.1), length(U/iResolution.xy));}"];
RenderShadertoy[shaderId, Format -> "Luminance"]
```

Note that loading the package multiple times will create more linked processes.

### Octave

The default install target for Octave puts `shadertoy_octave.oct` in the load
path. Note that `autoload` commands are needed to tell Octave to load the
functions from the correct file.

```matlab
% Tell octave to autoload the functions from the .oct file
shadertoy_oct = 'shadertoy_octave.oct';
autoload('st_render', shadertoy_oct);
autoload('st_compile', shadertoy_oct);

% Render a shadertoy as a matrix
img = st_render('llySRh');

% Show it
imshow(img);

% Grayscale render, next frame
img = st_render('llySRh', -1, 'Luminance');
```

## Author

Vincent Tavernier <vince.tavernier@gmail.com>

```
License: MIT
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```
