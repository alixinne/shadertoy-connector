#!/usr/bin/env perl
use strict;
use warnings;
use FindBin;
use lib "$FindBin::Bin/";
use StHelpers;
use Test::Simple tests => 1;

octave_ok 'Octave basic shader', <<OCTAVE_CODE
shadertoy_octave();
ctxt = st_compile('void mainImage(out vec4 O, in vec2 U){O=vec4(iFrame, iResolution.x, U.xy);}');
img = st_render(ctxt, 0, 'RGBA', 2, 2);
exit(ifelse(all(diag(img(1,1,:)(:) == [0.0 2.0 0.5 1.5])),0,2))
OCTAVE_CODE
