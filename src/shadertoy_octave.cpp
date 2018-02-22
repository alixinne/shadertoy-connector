#include "api.hpp"

static OMWrapper<OMWT_OCTAVE> wrapper([]() { host.Allocate(); });

// Function predeclaration
void oct_autoload(const std::string &);

// Locates the current .oct file, from https://stackoverflow.com/q/1681060
#include <dlfcn.h>
class oct_locator_class
{
	std::string _path;

	public:
	oct_locator_class()
	{
		Dl_info dl_info;
		dladdr((void *)oct_autoload, &dl_info);
		_path = std::string(dl_info.dli_fname);
	}

	inline const std::string &path() const { return _path; }
} oct_locator_default;

void oct_autoload(const std::string &fname)
{
	octave_value_list args;
	args(0) = fname;
	args(1) = oct_locator_default.path();

	feval("autoload", args);
}

DEFUN_DLD(shadertoy_octave, args, , "shadertoy_octave() initializes the shadertoy oct file")
{
	wrapper.CheckInitialization();

	oct_autoload("st_render");
	oct_autoload("st_reset");
	oct_autoload("st_compile");
	oct_autoload("st_set_input");
	oct_autoload("st_set_input_filter");
	oct_autoload("st_reset_input");

	return octave_value();
}

octave_value_list st_octave_run(const octave_value_list &args, std::function<void(OMWrapper<OMWT_OCTAVE> &)> fun)
{
	return st_wrapper_exec<OMWrapper<OMWT_OCTAVE>, const octave_value_list &>(wrapper, fun, args);
}

DEFUN_DLD(st_render, args, , "st_render('id', [frame, [format, [width, [height, "
							 "[mouse]]]]]) renders a Shadertoy as an image")
{
	return st_octave_run(args, impl_st_render<OMWrapper<OMWT_OCTAVE>>);
}

DEFUN_DLD(st_reset, args, , "st_reset('id') resets a context")
{
	return st_octave_run(args, impl_st_reset<OMWrapper<OMWT_OCTAVE>>);
}

DEFUN_DLD(st_compile, args, ,
		  "st_compile('source') compiles the source of a program and returns its id for st_render")
{
	return st_octave_run(args, impl_st_compile<OMWrapper<OMWT_OCTAVE>>);
}

DEFUN_DLD(st_set_input, args, ,
		  "st_set_input('id', 'image.0', matrix1[, 'image.1', matrix2[, ...]]])")
{
	return st_octave_run(args, impl_st_set_input<OMWrapper<OMWT_OCTAVE>>);
}

DEFUN_DLD(st_set_input_filter, args, ,
		  "st_set_input('id', 'image.0', 'linear'[, 'image.1', 'nearest'[, ...]]])")
{
	return st_octave_run(args, impl_st_set_input_filter<OMWrapper<OMWT_OCTAVE>>);
}

DEFUN_DLD(st_reset_input, args, , "st_reset_input('id', 'image.0'[, 'image.1'[, ...]]])")
{
	return st_octave_run(args, impl_st_reset_input<OMWrapper<OMWT_OCTAVE>>);
}
