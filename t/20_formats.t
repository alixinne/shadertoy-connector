#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/../ext/omw/t/";
use TestHelpers;
use Test::More tests => 6;

my $shader = <<GLSL;
void mainImage(out vec4 O, in vec2 U){O=vec4(iFrame, iResolution.x, U.xy);}
GLSL
$shader =~ s/\n//g;

octave_ok 'RGBA format', <<OCTAVE_CODE;
img = st_render(st_compile("$shader"), 0, 2, 2, 'rgba');
exit(ifelse(all(diag(img(1,1,:)(:) == [0.0 2.0 0.5 1.5])),0,2))
OCTAVE_CODE

mathematica_ok 'RGBA format', <<MATHEMATICA_CODE;
img = ImageData[RenderShadertoy[CompileShadertoy["$shader"], Frame -> 0, Size -> { 2, 2 }, Format -> "RGBA"]];
Assert[img[[1, 1]] == {0.0, 2.0, 0.5, 1.5}]
MATHEMATICA_CODE

octave_ok 'RGB format', <<OCTAVE_CODE;
img = st_render(st_compile("$shader"), 0, 2, 2, 'rgb');
exit(ifelse(all(diag(img(1,1,:)(:) == [0.0 2.0 0.5])),0,2))
OCTAVE_CODE

mathematica_ok 'RGB format', <<MATHEMATICA_CODE;
img = ImageData[RenderShadertoy[CompileShadertoy["$shader"], Frame -> 0, Size -> { 2, 2 }, Format -> "RGB"]];
Assert[img[[1, 1]] == {0.0, 2.0, 0.5}]
MATHEMATICA_CODE

octave_ok 'Luminance format', <<OCTAVE_CODE;
img = st_render(st_compile("$shader"), 0, 2, 2, 'luminance');
exit(ifelse(all(diag(img(1,1,1)(:) == [0.0])),0,2))
OCTAVE_CODE

mathematica_ok 'Luminance', <<MATHEMATICA_CODE;
img = ImageData[RenderShadertoy[CompileShadertoy["$shader"], Frame -> 0, Size -> { 2, 2 }, Format -> "Luminance"]];
Assert[img[[1, 1]] == 0.0]
MATHEMATICA_CODE
