# shadertoy-connector functions

<!-- MDTOC maxdepth:1 firsth1:0 numbering:0 flatten:0 bullets:1 updateOnSave:0 -->

- [Introduction](#introduction)   
- [Initialization](#initialization)   
- [st_compile: GLSL Compilation](#st_compile-glsl-compilation)   
- [st_render: Context rendering](#st_render-context-rendering)   
- [st_set_input: Set input texture](#st_set_input-set-input-texture)   
- [st_set_input_filter: Set input texture filter](#st_set_input_filter-set-input-texture-filter)   
- [st_reset_input: Reset input texture](#st_reset_input-reset-input-texture)   
- [st_reset: Reset context](#st_reset-reset-context)   

<!-- /MDTOC -->

## Introduction

Most functions in this toolkit operate on a *context*. A context defines the
fragment shaders, buffers and inputs that are used to render frames for a given
Shadertoy. It is identified by a string, which either refers to a local context
(ie. a context you have manually provided the sources for) or a shadertoy.com
context (ie. a context which has been/will be loaded from the shadertoy.com
API).

The function [st_compile/CompileShadertoy](#st_compile-glsl-compilation) can be
used to compile GLSL code to create a local context.
[st_set_input/SetShadertoyInput](#st_set_input-set-input-texture) can later be
used to set the inputs of the resulting context buffers.

If you use a Shadertoy id (from shadertoy.com) as a context id, and it is
accessible through the API (access level: *public + API*), it will be loaded
using the shadertoy.com API before rendering.

If the context id does not exist, or if it cannot be loaded from the
shadertoy.com API, an error message will be output to the standard output
(Octave) or as error messages (Mathematica).

## Initialization

### Synopsis

```
(* Mathematica *)
<<Shadertoy`

% Octave
shadertoy_octave();
```

### Description

Initializes the Shadertoy Connector library.

### Arguments

None

### Return value

None

## st_compile: GLSL Compilation

### Synopsis

```
(* Mathematica *)
code = "void mainImage(out vec4 O, in vec2 U){O = U.xyxy;}";
ctxt = CompileShadertoy[code];

% Octave
code = 'void mainImage(out vec4 O, in vec2 U){O = U.xyxy;}';
ctxt = st_compile(code);
```

### Description

Compiles a fragment shader as if it was the code for the image buffer of a
Shadertoy context. The return value is the identifier for the rendering context.

Any GLSL compilation errors will be output to the standard output (Octave) or as
error messages (Mathematica).

### Arguments

* `code`: GLSL fragment shader to compile, with mainImage as its entry point

### Return value

* `ctxt`: String that identifies the rendering context based on this fragment
shader

## st_render: Context rendering

### Synopsis

```
(* Mathematica *)
img = RenderShadertoy[ctxt, Frame -> Null, Size -> { 640, 360 }, Mouse ->
	Format -> "RGB", { 0, 0, 0, 0 }, FrameTiming -> False];

% Octave
img = st_render(ctxt, -1, 640, 360, 'RGB', [0 0 0 0], false);
```

### Description

Renders a single frame of the given context `ctxt`.

### Arguments

* `ctxt`: String that identifies the context to render
* *(optional)* `Frame` (Mathematica) or 2nd arg (Octave): Number of the frame to
render (`iFrame/iTime` uniforms). Use `Null` (Mathematica) or `-1` (Octave) to
render the next frame following the previous render call.
* *(optional)* `Size` (Mathematica) or 3rd (width) and 4th (height) args
(Octave): Size of the rendering viewport. If `Size` is a single number, a square
viewport will be assumed. Use `Null` (Mathematica) or `-1` (Octave) for the
default size.
* *(optional)* `Format` (Mathematica) or 5th arg (Octave): Format of the
rendering. Can either be `'Luminance'` (one-channel), `'RGB'` (three-channel) or
`'RGBA'` (four-channel). Defines the number of channels in the returned value.
* *(optional)* `Mouse` (Mathematica) or 6th arg (Octave): Value of the `iMouse`
uniform, as a 2 or 4 component vector of floats.
* *(optional)* `FrameTiming` (Mathematica) or 7th arg (Octave): set to `True` to
return a list containing the running time of the shader, queried using
glBeginQuery(GL_TIMESTAMP), and the rendered image. Defaults to `False`
(only return the rendered image).

### Return value

In Octave, calling this function either returns a HxWxD matrix, where H, W are
the requested height and width of the rendering, and D is the number of channels
of the requested format (1, 3 or 4).

In Mathematica, calling this function returns a floating-point image object of
size HxW, with D channels, where D is the number of channels of the requested
format (1, 3 or 4).

In Mathematica, if `FrameTiming` is set to `True`, a list will be returned
instead. The first element will be the runtime of the image buffer fragment
shader invocation, in seconds. The second element will be the rendered image.

## st_set_input: Set input texture

### Synopsis

```
(* Mathematica *)
input1 = ExampleData[{"TestImage", "Lena"}];
SetShadertoyInput[ctxt, "image.0" -> "a", "a.0" -> input1];

% Octave
input1 = imread("lena512.png");
st_set_input(ctxt, "image.0", "a", "a.0", input1);
```

### Description

Sets the textures that will be used to render frames from the context `ctxt`. If
`ctxt` is the identifier of a remote context, this function overrides the inputs
specified on the shadertoy.com website.

An error will be raised if the input identifier is invalid, either because it is
malformed, or because the named buffer does not exist.

### Arguments

* `ctxt`: String that identifies the context.
* *(may occur many times)* `InputName -> InputImage` (Mathematica) or
`InputName, InputMatrix` (Octave): Sets the texture to use for the input named
`InputName`. `InputName` has the form `bufferName.channel` where `bufferName` is
one of the buffers defined in the context named `ctxt`, and `channel` is in the
inclusive range 0-3. `InputImage` must be a gray-level, RGB or RGBA image
object. `InputMatrix` must be a HxWxD matrix representing either a gray-level,
RGB or RGBA image.
* *(may occur many times)* `InputName -> BufferName` (Mathematica) or
`InputName, BufferName` (Octave): Sets the texture to use for the input named
`InputName`. `InputName` is defined as above. `BufferName` is the name of one
of the buffers defined in the context `ctxt`.

### Return value

None.

### Additional notes

*libshadertoy* uses GL_FLOAT/GL_RGBA32F textures internally, which means aside
from the conversion of matrix elements to single-precision floating point
numbers, no other changes are made to the inputs before being passed to the
driver. If you want 8-bit textures to be scaled between 0 and 1, this has to
be done manually, before calling `st_set_input/SetShadertoyInput`.

## st_set_input_filter: Set input texture filter

### Synopsis

```
(* Mathematica *)
SetShadertoyInputFilter[ctxt, "image.0" -> "Linear", "a.0" -> "Mipmap"];

% Octave
st_set_input_filter(ctxt, 'image.0', 'Linear', 'a.0', 'Mipmap');
```

### Description

Sets the texture unit filter to be used when rendering frames from the context
`ctxt`. If `ctxt` is the identifier of a remote context, this function overrides
the input filters specified on the shadertoy.com website.

An error will be raised if the input identifier is invalid, either because it is
malformed, or because the named buffer does not exist.

### Arguments

* `ctxt`: String that identifies the context.
* *(may occur many times)* `InputName -> InputFilter` (Mathematica) or
`InputName, InputFilter` (Octave): Sets the texture filter to use for the input
named `InputName`. `InputName` has the form `bufferName.channel` where
`bufferName` is one of the buffers defined in the context named `ctxt`, and
`channel` is in the inclusive range 0-3. `InputImage` must be a string
identifying the filtering mode to use, either `'Nearest'`, `'Linear'`, or
`'Mipmap'`.

### Return value

None.

## st_reset_input: Reset input texture

### Synopsis

```
(* Mathematica *)
ResetShadertoyInput[ctxt, "image.0", "a.0"];

% Octave
st_reset_input(ctxt, 'image.0', 'a.0');
```

### Description

When used with a remote context, resets the input properties (texture and
filter, as set by [st_set_input](#st_set_input-set-input-texture) and
[st_set_input_filter](#st_set_input_filter-set-input-texture-filter)) to the
defaults specified on the shadertoy.com website.

### Arguments

* `ctxt`: String that identifies the remote context.
* *(may occur many times)* `InputName`: Name of the buffer input to reset. See
[st_set_input/SetShadertoyInput](#st_set_input-set-input-texture) for details.

### Return value

None.

## st_reset: Reset context

### Synopsis

```
(* Mathematica *)
ResetShadertoy[ctxt];

% Octave
st_reset(ctxt);
```

### Description

When used with a remote context, clears all cached state. The next call
using this context id will reload the context from shadertoy.com.

### Arguments

* `ctxt`: String that identifies the remote context.

### Return value

None.
