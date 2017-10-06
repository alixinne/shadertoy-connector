void st_render P(( ));

:Begin:
:Function:       st_render
:Pattern:        RenderShadertoy[id_String, OptionsPattern[]]
:Arguments:      { id, OptionValue[Frame], OptionValue[Width], OptionValue[Height] }
:ArgumentTypes:  { Manual }
:ReturnType:     Manual
:End:

:Evaluate: RenderShadertoy::usage = "RenderShadertoy[id] renders a Shadertoy as an image";
:Evaluate: RenderShadertoy::glfwerr = "A GLFW error occurred: `1`";
:Evaluate: RenderShadertoy::glerr = "OpenGL error: `1`";
:Evaluate: RenderShadertoy::err = "Error: `1`";
:Evaluate: Options[RenderShadertoy] = { Frame -> Null, Width -> 640, Height -> 360 };
