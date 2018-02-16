# shadertoy-connector functions

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

## GLSL Compilation

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

## Context rendering

### Synopsis

```
(* Mathematica *)
img = RenderShadertoy[ctxt, Frame -> Null, Size -> { 640, 360 }, Mouse ->
	{ 0, 0, 0, 0 }, Format -> "RGB", FrameTiming -> False];

(* Octave *)
img = st_render(ctxt, -1, 'RGB', 640, 360, [0 0 0 0]);
```

### Description

Renders a single frame of the given context `ctxt`. `ctxt` can either be the
name of a local context compiled by the `st_compile/CompileShadertoy` function,
or the id of a Shadertoy that is accessible through the shadertoy.com API.

If the context does not exist, or if it cannot be loaded from the shadertoy.com
API, an error message will be output to the standard output (Octave) or as
error messages (Mathematica).

### Arguments

* `ctxt`: String that identifies the context to render
* `Frame` (Mathematica) or 2nd arg (Octave): Number of the frame to render
(`iFrame/iTime` uniforms). Use `Null` (Mathematica) or `-1` (Octave) to render
the next frame following the previous render call.
* `Size` (Mathematica) or 3rd and 4th args (Octave): Size of the rendering
viewport. If `Size` is a single number, a square viewport will be assumed.
* `Format` (Mathematica) or 5th arg (Octave): Format of the rendering. Can
either be `'Luminance'` (one-channel), `'RGB'` (three-channel) or `'RGBA'`
(four-channel). Defines the number of channels in the returned value.
* `Mouse` (Mathematica) or 6th arg (Octave): Value of the `iMouse` uniform, as
a 2 or 4 component vector of floats.
* `FrameTiming` (Mathematica only): set to `True` to return a list containing
the running time of the shader, queried using glBeginQuery(GL_TIMESTAMP), and
the rendered image. Defaults to `False` (only return the rendered image).

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
