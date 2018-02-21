#!/bin/bash

# Bash strict mode
set -u

# Test name
cd "$(dirname $BASH_SOURCE)"
source ./helpers.sh "$BASH_SOURCE"

# Test the library can be loaded (Octave) and a shader can compile and run
octave_test "Octave basic shader" <<OCTAVE_CODE
shadertoy_octave();
ctxt = st_compile('void mainImage(out vec4 O, in vec2 U){O=vec4(iFrame, iResolution.x, U.xy);}');
img = st_render(ctxt, 0, 'RGBA', 2, 2);
octave_tap(all(diag(img(1,1,:)(:) == [0.0 2.0 0.5 1.5])), 'TEST_MESSAGE')
OCTAVE_CODE

test_done
