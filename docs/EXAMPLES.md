# shadertoy-connector examples

<!-- MDTOC maxdepth:2 firsth1:0 numbering:0 flatten:0 bullets:1 updateOnSave:1 -->

- [Mathematica](#Mathematica)   
- [Octave](#Octave)   

<!-- /MDTOC -->

## Mathematica

If you followed the installation instructions, the Shadertoy package will be in
the user package load path, so it can be loaded directly from any session.

```mathematica
(* Load the Shadertoy package *)
<< Shadertoy`

(* Render a specific frame of a Shadertoy, with given inputs *)
Manipulate[
	RenderShadertoy["llySRh", Frame -> n, Mouse -> {x, y}],
		{n, 0, 10, 1}, {x, 0, 640}, {y, 0, 360}]

(* Compile some GLSL source and render it *)
shaderId = CompileShadertoy["void mainImage(out vec4 O, in vec2 U){O = vec4(sin(iTime*.1), cos(iTime*.1), length(U/iResolution.xy));}"];
RenderShadertoy[shaderId, Format -> "Luminance"]
```

Note that loading the package multiple times will terminate previous linked
processes and start them again, therefore clearing any state held by the various
contexts.

## Octave

The default install target for Octave puts `shadertoy_octave.oct` in the load
path.

```matlab
% Tell octave to autoload the functions from the .oct file
shadertoy_octave();

% Render a shadertoy as a matrix
img = st_render('llySRh');

% Show it
% Note that Octave does not support RGBA images, thus we only feed RGB to imshow
imshow(img(:,:,1:3));

% Grayscale render, next frame
img = st_render('llySRh', -1, -1, -1, 'Luminance');
```
