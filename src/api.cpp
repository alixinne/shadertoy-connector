#include "api.hpp"

/// Shadertoy Connector context host
Host host;

std::string mathematica_unescape(const std::string &source)
{
	// Process escapes
	std::stringstream unescaped;
	size_t len = source.size();
	enum
	{
		Standard,
		ReadingEscape,
		ReadingOctalEscape
	} state = Standard;
	int cnum;

	for (size_t i = 0; i <= len; ++i)
	{
		if (state == Standard)
		{
			if (source[i] == '\\')
			{
				state = ReadingEscape;
				cnum = 0;
			}
			else if (source[i])
			{
				unescaped << source[i];
			}
		}
		else if (state == ReadingEscape)
		{
			if (source[i] == '0')
			{
				state = ReadingOctalEscape;
			}
			else if (source[i] == 'n')
			{
				unescaped << '\n';
				state = Standard;
			}
			else if (source[i] == 'r')
			{
				unescaped << '\r';
				state = Standard;
			}
			else if (source[i] == 't')
			{
				unescaped << '\t';
				state = Standard;
			}
			else
			{
				unescaped << '\\';
				unescaped << source[i];
				state = Standard;
			}
		}
		else if (state == ReadingOctalEscape)
		{
			if (source[i] >= '0' && source[i] <= '7')
			{
				cnum = cnum * 8 + (source[i] - '0');
			}
			else
			{
				unescaped << static_cast<char>(cnum);
				state = Standard;
				i--;
			}
		}
	}

	return unescaped.str();
}

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
