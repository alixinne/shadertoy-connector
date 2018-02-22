void st_render P(( ));

:Begin:
:Function:       st_render
:Pattern:        RenderShadertoy[id_String, OptionsPattern[]]
:Arguments:      { id, OptionValue[Frame], With[{ size = OptionValue[Size] }, If[ListQ[size], size[[1]], size]], With[{ size = OptionValue[Size] }, If[ListQ[size], size[[2]], size]], OptionValue[Mouse], OptionValue[Format], OptionValue[FrameTiming] }
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

:Evaluate: RenderShadertoy::usage = "RenderShadertoy[id] renders a Shadertoy as an image";
:Evaluate: Options[RenderShadertoy] = { Frame -> Null, Size -> { 640, 360 }, Mouse -> { 0, 0, 0, 0 }, Format -> "RGB", FrameTiming -> False };

void st_reset P((const char *));

:Begin:
:Function:       st_reset
:Pattern:        ResetShadertoy[id_String]
:Arguments:      { id }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

:Evaluate: ResetShadertoy::usage = "ResetShadertoy[id] resets the rendering context of a Shadertoy";

void st_compile P((const char *));

:Begin:
:Function:      st_compile
:Pattern:       CompileShadertoy[source_String]
:Arguments:     { source }
:ArgumentTypes: { String }
:ReturnType:    Manual
:End:

:Evaluate: CompileShadertoy::usage = "CompileShadertoy[source] compiles source as a Shadertoy and returns its id";

:Evaluate: Shadertoy::glerr = "OpenGL error: `1`";
:Evaluate: Shadertoy::err = "Error: `1`";

void st_set_input P(( ));

:Begin:
:Function:      st_set_input
:Pattern:       SetShadertoyInput[id_String, inputs__Rule]
:Arguments:     { id, Map[{#[[1]], ImageData[#[[2]]]} &, List[inputs]] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: SetShadertoyInput::usage = "SetShadertoyInput[id, input -> image] sets the input 'input' of the Shadertoy context 'id' to 'image'";

void st_set_input_filter P(( ));

:Begin:
:Function:      st_set_input_filter
:Pattern:       SetShadertoyInputFilter[id_String, inputs__Rule]
:Arguments:     { id, Map[{#[[1]], #[[2]]} &, List[inputs]] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: SetShadertoyInputFilter::usage = "SetShadertoyInputFilter[id, input -> \"Linear\"|\"Nearest\"|\"Mipmap\"] sets the input 'input' filter method of the Shadertoy context 'id' to the specified value";

void st_reset_input P(( ));

:Begin:
:Function:      st_reset_input
:Pattern:       ResetShadertoyInput[id_String, inputs__String]
:Arguments:     { id, List[inputs] }
:ArgumentTypes: { Manual }
:ReturnType:    Manual
:End:

:Evaluate: SetShadertoyInput::usage = "ResetShadertoyInput[id, input, image] sets the input 'input' of the Shadertoy context 'id' to what was used by the shadertoy beforehand";