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
$ make -j32
```

## Usage

### Mathematica

In order to use this package in Mathematica, the parent folder (ie. the one
containing the `shadertoy-connector` package) must exist and be named `Shadertoy`.
It must also be in one of the folders which are in the Mathematica load path.

```mathematica
(* Add the package folder to the path, edit accordingly *)
AppendTo[$Path, FileNameJoin[{$UserDocumentsDirectory, "Wolfram Mathematica"}]];

(* Load the package *)
<< Shadertoy`

(* Render a specific frame of a Shadertoy, with given inputs *)
Manipulate[
	RenderShadertoy["llySRh", Frame -> n, Mouse -> {x, y}],
		{n, 0, 10, 1}, {x, 0, 640}, {y, 0, 360}];

(* Compile some GLSL source and render it *)
shaderId = CompileShadertoy["void mainImage(out vec4 O, in vec2 U){O = vec4(length(U/iResolution.xy));}"];
RenderShadertoy[shaderId]
```

Note that loading the package multiple times will create more linked processes.

### Octave

Ensure the built `shadertoy_octave.oct` file is somewhere in the load path. Here
we assume it is in the current directory, but change the path accordingly. You
can use the `make install` command in the `build/` directory to install the `oct`
file into the package directory.

```matlab
% Tell octave to autoload the functions from the .oct file
shadertoy_oct = 'shadertoy_octave.oct';
autoload('st_render', shadertoy_oct);
autoload('st_compile', shadertoy_oct);

% Render a shadertoy as a matrix
img = st_render('llySRh');

% Show it
imshow(img);
```

## Author

Vincent Tavernier <vince.tavernier@gmail.com>
