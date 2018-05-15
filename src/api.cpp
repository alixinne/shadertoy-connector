#include "api.hpp"

/// Shadertoy Connector context host
Host host;

bool impl_st_parse_input(std::string &inputSpecName, std::string &buffer, int &channel)
{
	auto dotPos(inputSpecName.find('.'));
	if (dotPos == std::string::npos)
	{
		std::stringstream ss;
		ss << "Invalid input specification " << inputSpecName;
		throw std::runtime_error(ss.str());
	}

	buffer.assign(inputSpecName.begin(), inputSpecName.begin() + dotPos);
	std::string inputChannelStr(inputSpecName.begin() + dotPos + 1, inputSpecName.end());
	channel = std::atoi(inputChannelStr.c_str());

	if (channel < 0 || channel > 3)
	{
		std::stringstream ss;
		ss << "Invalid channel number " << inputChannelStr;
		throw std::runtime_error(ss.str());
	}

	return true;
}

#if OMW_OCTAVE

static omw::octave wrapper(reinterpret_cast<void*>(&impl_st_parse_input),
	[]() { host.Allocate(); });

DEFUN_DLD(shadertoy_octave, args, , "shadertoy_octave() initializes the shadertoy oct file")
{
	wrapper.check_initialization();

	wrapper.set_autoload("st_render");
	wrapper.set_autoload("st_reset");
	wrapper.set_autoload("st_compile");
	wrapper.set_autoload("st_set_input");
	wrapper.set_autoload("st_set_input_filter");
	wrapper.set_autoload("st_reset_input");

	return octave_value();
}

octave_value_list st_octave_run(const octave_value_list &args,
	std::function<void(omw::octave&)> fun)
{
	return st_wrapper_exec<omw::octave, const octave_value_list &>(wrapper, fun, args);
}

#define OM_DEFUN(name,oct_usage) \
	DEFUN_DLD(name, args, , oct_usage) \
	{ \
		return st_octave_run(args, impl_ ## name <omw::octave>); \
	}

#endif /* OMW_OCTAVE */

#if OMW_MATHEMATICA

// Mathematica API wrapper
static omw::mathematica wrapper("Shadertoy", stdlink, []() { host.Allocate(); });

#define OM_DEFUN(name,oct_usage) \
	extern "C" void name (); \
	void name () \
 	{ \
		st_wrapper_exec<omw::mathematica>(wrapper, impl_ ## name <omw::mathematica>); \
	}

#endif /* OMW_MATHEMATICA */

#if !defined(OM_DEFUN)

#define OM_DEFUN(name,oct_usage)

#endif /* !defined(OM_DEFUN) */

OM_DEFUN(st_render, "st_render('id', [frame, [width, [height, [format, [mouse, [timing]]]]]]) renders a Shadertoy as an image")

OM_DEFUN(st_reset, "st_reset('id') resets a context")

OM_DEFUN(st_compile, "st_compile('source', 'a', 'sourceA') compiles the source of a program and "
					 "returns its id for st_render")

OM_DEFUN(st_set_input, "st_set_input('id', 'image.0', matrix1[, 'image.1', matrix2[, ...]]])")

OM_DEFUN(st_set_input_filter, "st_set_input('id', 'image.0', 'linear'[, 'image.1', 'nearest'[, ...]]])")

OM_DEFUN(st_reset_input, "st_reset_input('id', 'image.0'[, 'image.1'[, ...]]])")
