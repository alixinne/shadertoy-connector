void st_render P(( ));

:Begin:
:Function:       st_render
:Pattern:        RenderShadertoy[id_String, OptionsPattern[]]
:Arguments:      { id, OptionValue[Frame], With[{ size = OptionValue[Size] }, If[ListQ[size], size[[1]], size]], With[{ size = OptionValue[Size] }, If[ListQ[size], size[[2]], size]], OptionValue[Format], OptionValue[Mouse], OptionValue[FrameTiming] }
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

:Evaluate: RenderShadertoy::usage = "RenderShadertoy[id, Frame -> Null, Size -> { 640, 360 }, Mouse -> { 0, 0, 0, 0 }, FrameTiming -> False] renders a Shadertoy as an image";
:Evaluate: Options[RenderShadertoy] = { Frame -> Null, Size -> { 640, 360 }, Mouse -> { 0, 0, 0, 0 }, Format -> "RGB", FrameTiming -> False };

:Evaluate: Size        = Symbol["Size"];
:Evaluate: Mouse       = Symbol["Mouse"];
:Evaluate: FrameTiming = Symbol["FrameTiming"];

void st_reset P(( ));

:Begin:
:Function:       st_reset
:Pattern:        ResetShadertoy[id_String]
:Arguments:      { id }
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

:Evaluate: ResetShadertoy::usage = "ResetShadertoy[id] resets the rendering context of a Shadertoy";

void st_compile P(( ));

:Begin:
:Function:      st_compile
:Pattern:       CompileShadertoy[source_String, buffers__Rule | PatternSequence[]]
:Arguments:     { source, Map[{#[[1]], #[[2]]} &, List[buffers]] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: CompileShadertoy::usage = "CompileShadertoy[source, \"a\" -> sourceA] compiles source as a Shadertoy and returns its id";

:Evaluate: Shadertoy::glerr = "OpenGL error: `1`";
:Evaluate: Shadertoy::err = "Error: `1`";

void st_set_input P(( ));

:Begin:
:Function:      st_set_input
:Pattern:       SetShadertoyInput[id_String, inputs__Rule]
:Arguments:     { id, Map[{#[[1]], If[StringQ[#[[2]]], #[[2]], ImageData[#[[2]]]]} &, List[inputs]] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: SetShadertoyInput::usage = "SetShadertoyInput[id, input -> image|\"buffer\"] sets the input 'input' of the Shadertoy context 'id' to 'image' or to the output of the named buffer \"buffer\"";

void st_set_input_filter P(( ));

:Begin:
:Function:      st_set_input_filter
:Pattern:       SetShadertoyInputFilter[id_String, inputs__Rule]
:Arguments:     { id, Map[{#[[1]], If[StringQ[#[[2]]],#[[2]],SymbolName[#[[2]]]]} &, List[inputs]] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: Linear  = Symbol["Linear"];
:Evaluate: Mipmap  = Symbol["Mipmap"];

:Evaluate: SetShadertoyInputFilter::usage = "SetShadertoyInputFilter[id, input -> Linear|Nearest|Mipmap] sets the input 'input' filter method of the Shadertoy context 'id' to the specified value";

void st_reset_input P(( ));

:Begin:
:Function:      st_reset_input
:Pattern:       ResetShadertoyInput[id_String, inputs__String]
:Arguments:     { id, List[inputs] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: SetShadertoyInput::usage = "ResetShadertoyInput[id, input, image] sets the input 'input' of the Shadertoy context 'id' to what was used by the shadertoy beforehand";
