#include "api.hpp"

extern "C" {
void st_compile();
void st_reset();
void st_render();

void st_set_input();
void st_reset_input();

void st_set_input_filter();
}

// Mathematica API wrapper
static OMWrapper<OMWT_MATHEMATICA> wrapper("Shadertoy", stdlink, []() { host.Allocate(); });

void st_compile()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_compile<OMWrapper<OMWT_MATHEMATICA>>);
}

void st_reset()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_reset<OMWrapper<OMWT_MATHEMATICA>>);
}

void st_render()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_render<OMWrapper<OMWT_MATHEMATICA>>);
}

void st_set_input()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_set_input<OMWrapper<OMWT_MATHEMATICA>>);
}

void st_reset_input()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_reset_input<OMWrapper<OMWT_MATHEMATICA>>);
}

void st_set_input_filter()
{
	st_wrapper_exec<OMWrapper<OMWT_MATHEMATICA>>(wrapper, impl_st_set_input_filter<OMWrapper<OMWT_MATHEMATICA>>);
}
