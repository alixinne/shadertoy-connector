void st_render P(( const char* ));

:Begin:
:Function:       st_render
:Pattern:        RenderShadertoy[id_String]
:Arguments:      { id }
:ArgumentTypes:  { String }
:ReturnType:     Manual
:End:

:Evaluate: RenderShadertoy::usage = "RenderShadertoy[id] renders a Shadertoy as an image";
:Evaluate: RenderShadertoy::glfwerr = "A GLFW error occurred: `1`";
:Evaluate: RenderShadertoy::glerr = "OpenGL error: `1`";
:Evaluate: RenderShadertoy::err = "Generic error: `1`";
