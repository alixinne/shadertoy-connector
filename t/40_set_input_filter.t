#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/";
use StHelpers;
use Test::More tests => 2;

my $shader = <<GLSL;
void mainImage(out vec4 O, in vec2 U){O=texture(iChannel0, U / iResolution.xy);}
GLSL
$shader =~ s/\n//g;

octave_ok 'Passthrough texture test', <<OCTAVE_CODE;
m = [0 1];
ctxt = st_compile("$shader");
st_set_input(ctxt, "image.0", m);
st_set_input_filter(ctxt, "image.0", "Linear");
imgl = st_render(ctxt, 0, 1, 1, 'Luminance');
st_set_input_filter(ctxt, "image.0", "Nearest");
imgn = st_render(ctxt, 0, 1, 1, 'Luminance');
disp("imgl")
disp(imgl)
disp("imgn")
disp(imgn)
exit(ifelse(imgn==1 && imgl==0.5,0,1));
OCTAVE_CODE

mathematica_ok 'Passthrough texture test', <<MATHEMATICA_CODE;
m = {{0, 1}};
ctxt = CompileShadertoy["$shader"];
SetShadertoyInput[ctxt, "image.0" -> Image[m]];
SetShadertoyInputFilter[ctxt, "image.0" -> "Linear"];
imgl = RenderShadertoy[ctxt, Format -> "Luminance", Size -> 1];
SetShadertoyInputFilter[ctxt, "image.0" -> "Nearest"];
imgn = RenderShadertoy[ctxt, Format -> "Luminance", Size -> 1];
Print[MatrixForm[ImageData[imgl]]];
Print[MatrixForm[ImageData[imgn]]];
Assert[ImageData[imgn][[1, 1]] == 1. && ImageData[imgl][[1, 1]] == .5]
MATHEMATICA_CODE

