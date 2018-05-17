#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../ext/omw/t/";
use TestHelpers;
use Test::More tests => 2;

my $shaderImage = <<GLSL;
void mainImage(out vec4 O, in vec2 U){O=texelFetch(iChannel0, ivec2(U-.5), 0);}
GLSL

my $shaderA = <<GLSL;
void mainImage(out vec4 O, in vec2 U){O=vec4(1., 2., 3., 4.);}
GLSL

$shaderImage =~ s/\n//g;
$shaderA =~ s/\n//g;

octave_ok 'Basic shader', <<OCTAVE_CODE;
ctxt = st_compile("$shaderImage", "a", "$shaderA");
st_set_input(ctxt, "image.0", "a");
img = st_render(ctxt, 0, 1, 1, 'rgba');
disp(img)
exit(ifelse(all(diag(img(1,1,:)(:) == [1. 2. 3. 4.])),0,1))
OCTAVE_CODE

mathematica_ok 'Basic shader', <<MATHEMATICA_CODE;
ctxt = CompileShadertoy["$shaderImage", "a" -> "$shaderA"];
SetShadertoyInput[ctxt, "image.0" -> "a"];
img = ImageData[RenderShadertoy[ctxt, Frame -> 0, Size -> 1, Format -> "RGBA"]];
Print[img];
Assert[img[[1, 1]] == {1., 2., 3., 4.}]
MATHEMATICA_CODE

