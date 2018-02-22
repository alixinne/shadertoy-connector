#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/";
use StHelpers;
use Test::More tests => 2;

my $shader = <<GLSL;
void mainImage(out vec4 O, in vec2 U){O=vec4(iFrame, iResolution.x, U.xy);}
GLSL
$shader =~ s/\n//g;

octave_ok 'Basic shader', <<OCTAVE_CODE;
ctxt = st_compile("$shader");
img = st_render(ctxt, 0, 'RGBA', 2, 2);
exit(ifelse(all(diag(img(1,1,:)(:) == [0.0 2.0 0.5 1.5])),0,2))
OCTAVE_CODE

mathematica_ok 'Basic shader', <<MATHEMATICA_CODE;
<<Shadertoy`;
ctxt = CompileShadertoy["$shader"];
img = RenderShadertoy[ctxt, Format -> "RGBA", Frame -> 0, Size -> { 2, 2 }];
Exit[If[img[[1, 1]] == {0.0, 2.0, 0.5, 1.5},0,1]];
MATHEMATICA_CODE
