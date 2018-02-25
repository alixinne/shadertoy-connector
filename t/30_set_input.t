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
mb = [1, 2, 3; 4, 5, 6];
m = zeros(2, 3, 4);
m(:,:,1) = mb;
m(:,:,2) = 10*mb;
m(:,:,3) = 100*mb;
m(:,:,4) = 1000*mb;
ctxt = st_compile("$shader");
st_set_input(ctxt, "image.0", m);
img = st_render(ctxt, 0, 'RGBA', 3, 2);
disp("m")
disp(m)
disp("img")
disp(img)
disp("m == img")
disp(img == m)
exit(ifelse(all(img==m),0,1));
OCTAVE_CODE

mathematica_ok 'Passthrough texture test', <<MATHEMATICA_CODE;
m = {{{1, 10, 100, 1000}, {2, 20, 200, 2000}, {3, 30, 300, 3000}},
	 {{4, 40, 400, 4000}, {5, 50, 500, 5000}, {6, 60, 600, 6000}}};
ctxt = CompileShadertoy["$shader"];
SetShadertoyInput[ctxt, "image.0" -> Image[m]];
img = RenderShadertoy[ctxt, Format -> "RGBA", Size -> { 3, 2 }];
Print[MatrixForm[ImageData[img]]];
Print[MatrixForm[m]];
Assert[ImageData[img]==m]
MATHEMATICA_CODE

