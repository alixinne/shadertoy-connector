#include <functional>
#include <sstream>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include "context.hpp"
#include "host.hpp"

#include "om_wrapper.h"

using namespace std;

extern "C" {
void st_compile(const char *source);
void st_reset(const char *id);
void st_render();

void st_set_input();
void st_reset_input();

void st_set_input_filter();
}

// Render context host
static Host host;

// Mathematica API wrapper
static OMWrapper<OMWT_MATHEMATICA>
wrapper("Shadertoy", stdlink, function<void(void)>([]() { host.Allocate(); }));


// Failure callback
void st_fail(const char *msgname, const char *arg)
{
	MLPutFunction(stdlink, "CompoundExpression", 2);
	MLPutFunction(stdlink, "Message", 2);
	MLPutFunction(stdlink, "MessageName", 2);
	MLPutSymbol(stdlink, "Shadertoy");
	MLPutString(stdlink, msgname);
	MLPutString(stdlink, arg);
	MLPutSymbol(stdlink, "$Failed");
}

template <typename TRet> TRet st_wrapper_exec(function<TRet(void)> &&fun)
{
	try
	{
		return fun();
	}
	catch (shadertoy::OpenGL::ShaderCompilationError &ex)
	{
		stringstream ss;
		ss << "Shader compilation error: " << ex.what() << endl << ex.log();
		st_fail("glerr", ss.str().c_str());
	}
	catch (shadertoy::OpenGL::ProgramLinkError &ex)
	{
		stringstream ss;
		ss << "Program link error: " << ex.what() << endl << ex.log();
		st_fail("glerr", ss.str().c_str());
	}
	catch (shadertoy::OpenGL::OpenGLError &ex)
	{
		stringstream ss;
		ss << "OpenGL error: " << ex.what();
		st_fail("glerr", ss.str().c_str());
	}
	catch (shadertoy::ShadertoyError &ex)
	{
		stringstream ss;
		ss << "Shadertoy error: " << ex.what();
		st_fail("err", ss.str().c_str());
	}
	catch (runtime_error &ex)
	{
		st_fail("err", ex.what());
	}

	return TRet();
}

template <> void st_wrapper_exec(function<void(void)> &&fun)
{
	st_wrapper_exec<bool>(function<bool(void)>([&fun]() {
		fun();
		return true;
	}));
}

void st_compile(const char *source)
{
	st_wrapper_exec(function<void(void)>([&]() {
		// Process escapes
		std::stringstream unescaped;
		size_t len = strlen(source);
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

		string shaderId(host.CreateLocal(unescaped.str()));
		MLPutString(stdlink, shaderId.c_str());
	}));
}

void st_reset(const char *id)
{
	st_wrapper_exec(function<void(void)>([&]() {
		host.Reset(id);
		MLPutSymbol(stdlink, "True");
	}));
}

void st_render()
{
	const char *id_c;
	MLGetString(stdlink, &id_c);

	string id(id_c);
	MLReleaseString(stdlink, id_c);

	st_wrapper_exec(function<void(void)>([&]() {
		boost::optional<int> frameCount;

		int i;
		if (!MLGetInteger32(stdlink, &i))
		{
			MLClearError(stdlink);

			const char *symbol;
			if (MLGetSymbol(stdlink, &symbol))
			{
				// Null is default value
				bool is_null = strcmp("Null", symbol) == 0;

				MLReleaseSymbol(stdlink, symbol);

				if (!is_null)
					throw runtime_error("Invalid value for Frame");
			}
			else
			{
				MLClearError(stdlink);
				throw runtime_error("Could not get value for Frame");
			}
		}
		else
		{
			frameCount = boost::optional<int>(i);
		}

		// Read size params
		int width, height;
		MLGetInteger32(stdlink, &width);
		MLGetInteger32(stdlink, &height);

		// Read mouse param
		float *mouseParam;
		int mouseParamSize;

		if (!MLGetReal32List(stdlink, &mouseParam, &mouseParamSize))
		{
			MLClearError(stdlink);
			throw runtime_error("Could not get value for Mouse");
		}

		if (mouseParamSize != 2 && mouseParamSize != 4)
		{
			MLReleaseReal32List(stdlink, mouseParam, mouseParamSize);
			throw runtime_error("Mouse must be a list of size 2 or 4");
		}

		float mouse[4] = { 0.f, 0.f, 0.f, 0.f };
		memcpy(mouse, mouseParam, mouseParamSize * sizeof(float));
		MLReleaseReal32List(stdlink, mouseParam, mouseParamSize);

		// Read format
		const char *formatName;
		GLenum format;
		if (!MLGetString(stdlink, &formatName))
		{
			MLClearError(stdlink);
			throw runtime_error("Invalid Format parameter");
		}

		if (strcmp(formatName, "RGBA") == 0)
			format = GL_RGBA;
		else if (strcmp(formatName, "RGB") == 0)
			format = GL_RGB;
		else if (strcmp(formatName, "Luminance") == 0)
			format = GL_LUMINANCE;
		else
		{
			MLReleaseString(stdlink, formatName);
			throw runtime_error("Invalid Format parameter");
		}

		MLReleaseString(stdlink, formatName);

		// Read frame timing parameter
		const char *frameTiming;
		if (!MLGetSymbol(stdlink, &frameTiming))
		{
			MLClearError(stdlink);
			throw runtime_error("Invalid FrameTiming parameter");
		}

		bool doFrameTiming;
		if (strcmp(frameTiming, "True") == 0)
		{
			doFrameTiming = true;
		}
		else if (strcmp(frameTiming, "False") == 0)
		{
			doFrameTiming = false;
		}
		else
		{
			MLReleaseSymbol(stdlink, frameTiming);
			throw runtime_error("Invalid FrameTiming parameter");
		}

		MLReleaseSymbol(stdlink, frameTiming);

		// Render image
		auto image(host.Render(id, frameCount, width, height, mouse, format));

		if (doFrameTiming)
		{
			MLPutFunction(stdlink, "List", 2);
			MLPutReal64(stdlink, image->frameTiming / 1e9);
		}

		MLPutFunction(stdlink, "Image", 1);
		MLPutReal32Array(stdlink, image->data->data(), &image->dims[0], NULL, 3);
	}));
}

bool st_parse_input_name(string &buffer, int &channel)
{
	const char *inputSpec;
	if (!MLGetString(stdlink, &inputSpec))
	{
		MLClearError(stdlink);
		return false;
	}

	string inputStr(inputSpec);
	MLReleaseString(stdlink, inputSpec);

	auto dotPos(inputStr.find('.'));
	if (dotPos == string::npos)
		throw runtime_error("Invalid input specification");

	buffer.assign(inputStr.begin(), inputStr.begin() + dotPos);
	string inputChannelStr(inputStr.begin() + dotPos + 1, inputStr.end());
	channel = atoi(inputChannelStr.c_str());

	if (channel < 0 || channel > 3)
		throw runtime_error("Invalid channel number");

	return true;
}

bool st_parse_id(string &id)
{
	const char *idC;
	if (!MLGetString(stdlink, &idC))
	{
		MLClearError(stdlink);
		throw runtime_error("Failed to get context Id");
	}

	id = string(idC);
	MLReleaseString(stdlink, idC);
}

void st_set_input()
{
	st_wrapper_exec(function<void(void)>([&]() {
		string shaderId, bufferName;
		int channelName, cnt = 0;

		// Parse context id
		st_parse_id(shaderId);
		// Get the context
		auto context(host.GetContext(shaderId));

		long ninputs;
		if (!MLCheckFunction(stdlink, "List", &ninputs))
		{
			MLClearError(stdlink);
			throw runtime_error("Invalid input specification");
		}

		// Process all flattened rules
		for (long n = 0; n < ninputs; ++n)
		{
			long nargs;
			if (!MLCheckFunction(stdlink, "List", &nargs))
			{
				MLClearError(stdlink);
				throw runtime_error("Invalid input item specification");
			}

			if (!st_parse_input_name(bufferName, channelName))
				throw runtime_error("Could not parse input name");

			float *data;
			int *dims;
			int d;
			char **heads;

			if (!MLGetReal32Array(stdlink, &data, &dims, &heads, &d))
			{
				MLClearError(stdlink);

				stringstream ss;
				ss << "Failed to get image data for " << bufferName << "." << channelName;
				throw runtime_error(ss.str());
			}

			if (d <= 1 || d > 3)
			{
				MLReleaseReal32Array(stdlink, data, dims, heads, d);
				stringstream ss;
				ss << "Invalid number of dimensions for " << bufferName << "." << channelName
				   << ". Must be 2 or 3";
				throw runtime_error(ss.str());
			}

			// Move image data in StImage structure
			StImage img;
			img.dims[0] = dims[0];
			img.dims[1] = dims[1];
			if (d == 3)
				img.dims[2] = dims[2];
			else
				img.dims[2] = 1;
			img.data = make_shared<vector<float>>(img.dims[0] * img.dims[1] * img.dims[2]);

			// Copy data, vflip
			size_t stride_size = sizeof(float) * img.dims[1] * img.dims[2];
			for (int i = 0; i < img.dims[0]; ++i)
			{
				memcpy(&img.data->data()[i * stride_size / sizeof(float)],
					   &data[(img.dims[0] - i - 1) * stride_size / sizeof(float)], stride_size);
			}

			// Release data
			MLReleaseReal32Array(stdlink, data, dims, heads, d);

			// Set context input
			context->setInput(bufferName, channelName, img);
			cnt++;
		}

		// Put the number of set inputs
		MLPutInteger(stdlink, cnt);
	}));
}

void st_reset_input()
{
	st_wrapper_exec(function<void(void)>([&]() {
		string shaderId, bufferName;
		int channelName, cnt = 0;

		// Parse context id
		st_parse_id(shaderId);
		// Get the context
		auto context(host.GetContext(shaderId));

		long ninputs;
		if (!MLCheckFunction(stdlink, "List", &ninputs))
		{
			MLClearError(stdlink);
			throw runtime_error("Invalid input specification");
		}

		// Parse all input IDs
		for (long n = 0; n < ninputs; ++n)
		{
			if (!st_parse_input_name(bufferName, channelName))
				throw runtime_error("Invalid input item specification");

			context->resetInput(bufferName, channelName);
			cnt++;
		}

		// Put the number of reset inputs
		MLPutInteger(stdlink, cnt);
	}));
}

void st_set_input_filter()
{
	st_wrapper_exec(function<void(void)>([&]() {
		string shaderId, bufferName;
		int channelName, cnt = 0;

		// Parse context id
		st_parse_id(shaderId);
		// Get the context
		auto context(host.GetContext(shaderId));

		long ninputs;
		if (!MLCheckFunction(stdlink, "List", &ninputs))
		{
			MLClearError(stdlink);
			throw runtime_error("Invalid input specification");
		}

		// Process all flattened rules
		for (long n = 0; n < ninputs; ++n)
		{
			long nargs;
			if (!MLCheckFunction(stdlink, "List", &nargs))
			{
				MLClearError(stdlink);
				throw runtime_error("Invalid input item specification");
			}

			if (!st_parse_input_name(bufferName, channelName))
				throw runtime_error("Could not parse input name");

			const char *filterMethod;
			if (!MLGetString(stdlink, &filterMethod))
			{
				MLClearError(stdlink);

				stringstream ss;
				ss << "Failed to get filter type for " << bufferName << "." << channelName;
				throw runtime_error(ss.str());
			}

			GLint minFilter;

			if (strcmp(filterMethod, "Linear") == 0)
			{
				minFilter = GL_LINEAR;
			}
			else if (strcmp(filterMethod, "Nearest") == 0)
			{
				minFilter = GL_NEAREST;
			}
			else if (strcmp(filterMethod, "Mipmap") == 0)
			{
				minFilter = GL_LINEAR_MIPMAP_LINEAR;
			}
			else
			{
				MLReleaseString(stdlink, filterMethod);

				stringstream ss;
				ss << "Invalid filter type '" << filterMethod << "' for " << bufferName << "." << channelName;
				throw runtime_error(ss.str());
			}

			// Release data
			MLReleaseString(stdlink, filterMethod);

			// Set context input filter
			context->setInputFilter(bufferName, channelName, minFilter);
			cnt++;
		}

		// Put the number of set inputs
		MLPutInteger(stdlink, cnt);
	}));
}
