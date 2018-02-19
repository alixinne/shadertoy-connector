#ifndef _API_HPP_
#define _API_HPP_

#include <array>
#include <exception>
#include <functional>
#include <sstream>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy/Shadertoy.hpp>

#include "om_wrapper.h"

#include "context.hpp"
#include "host.hpp"

extern Host host;

template <typename TWrapper>
void st_wrapper_exec(TWrapper &wrapper, std::function<void(TWrapper &)> fun)
{
	wrapper.CheckInitialization();

	wrapper.template RunFunction([&fun](TWrapper &wrapper) {
		try
		{
			fun(wrapper);
		}
		catch (shadertoy::OpenGL::ShaderCompilationError &ex)
		{
			std::stringstream ss;
			ss << "Shader compilation error: " << ex.what() << std::endl << ex.log();
			wrapper.SendFailure(ss.str(), "glerr");
		}
		catch (shadertoy::OpenGL::ProgramLinkError &ex)
		{
			std::stringstream ss;
			ss << "Program link error: " << ex.what() << std::endl << ex.log();
			wrapper.SendFailure(ss.str(), "glerr");
		}
		catch (shadertoy::OpenGL::OpenGLError &ex)
		{
			std::stringstream ss;
			ss << "OpenGL error: " << ex.what();
			wrapper.SendFailure(ss.str(), "glerr");
		}
		catch (shadertoy::ShadertoyError &ex)
		{
			std::stringstream ss;
			ss << "Shadertoy error: " << ex.what();
			wrapper.SendFailure(ss.str());
		}
		catch (std::runtime_error &ex)
		{
			wrapper.SendFailure(ex.what());
		}
	});
}

template <typename TWrapper> void impl_st_compile(TWrapper &w)
{
	std::string source(w.template GetParam<std::string>(0, "code"));

	w.template ConditionalRun<OMWrapper<OMWT_MATHEMATICA>>([&source]() {
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

		source = unescaped.str();
	});

	std::string shaderId(host.CreateLocal(source));

	w.template EvaluateResult<OMWrapper<OMWT_MATHEMATICA>>(
	[&]() { MLPutString(w.link, shaderId.c_str()); });
}

template <typename TWrapper> void impl_st_reset(TWrapper &w)
{
	host.Reset(w.template GetParam<std::string>(0, "ctxt"));
}

template <typename TWrapper> void impl_st_render(TWrapper &w)
{
	auto id(w.template GetParam<std::string>(0, "ctxt"));

	auto frameCount(w.template GetOptionalParam<int>(1, "Frame"));

	auto width(w.template GetParam<int>(2, "Width"));
	auto height(w.template GetParam<int>(3, "Height"));

	auto mouse(w.template GetParam<std::shared_ptr<OMArray<float>>>(4, "Mouse"));

	auto formatName(w.template GetParam<std::string>(5, "Format"));
	GLenum format;

	if (formatName.compare("RGBA") == 0)
		format = GL_RGBA;
	else if (formatName.compare("RGB") == 0)
		format = GL_RGB;
	else if (formatName.compare("Luminance") == 0)
		format = GL_LUMINANCE;
	else
		throw std::runtime_error("Invalid Format parameter");

	auto doFrameTiming(w.template GetParam<bool>(6, "FrameTiming"));

	auto image(host.Render(id, frameCount, width, height, mouse->data(), format));

	w.template EvaluateResult<OMWrapper<OMWT_MATHEMATICA>>([&]() {
		if (doFrameTiming)
		{
			MLPutFunction(w.link, "List", 2);
			MLPutReal64(w.link, image->frameTiming / 1e9);
		}

		MLPutFunction(w.link, "Image", 1);
		MLPutReal32Array(w.link, image->data->data(), &image->dims[0], NULL, 3);
	});
}

bool impl_st_parse_input(std::string &inputSpecName, std::string &buffer, int &channel);

template <typename TWrapper> void impl_st_set_input(TWrapper &w)
{
	// Parse context id
	auto id(w.template GetParam<std::string>(0, "ctxt"));
	// Get context object
	auto context(host.GetContext(id));

	// Number of defined inputs
	long ninputs;

	// Get number of inputs (Mathematica)
	w.template ConditionalRun<OMWrapper<OMWT_MATHEMATICA>>([&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get <name, image> tuple
		auto inputSpec(
		w.template GetParam<std::string, std::shared_ptr<OMMatrix<float>>>(n + 1, "InputSpec"));

		// Parse name
		std::string bufferName;
		int channelName;
		impl_st_parse_input(std::get<0>(inputSpec), bufferName, channelName);

		// Get depth for tests
		int d = std::get<1>(inputSpec)->depth();

		// Check dimensions
		if (d <= 1 || d > 3)
		{
			std::stringstream ss;
			ss << "Invalid number of dimensions for " << std::get<0>(inputSpec)
			   << ". Must be 2 or 3";
			throw std::runtime_error(ss.str());
		}

		// Move image data in StImage structure
		StImage img;
		img.dims[0] = std::get<1>(inputSpec)->dims()[0];
		img.dims[1] = std::get<1>(inputSpec)->dims()[1];
		if (d == 3)
			img.dims[2] = std::get<1>(inputSpec)->dims()[2];
		else
			img.dims[2] = 1;
		img.data = std::make_shared<std::vector<float>>(img.dims[0] * img.dims[1] * img.dims[2]);

		// Copy data, vflip
		size_t stride_size = sizeof(float) * img.dims[1] * img.dims[2];
		for (int i = 0; i < img.dims[0]; ++i)
		{
			memcpy(&img.data->data()[i * stride_size / sizeof(float)],
				   &std::get<1>(inputSpec)->data()[(img.dims[0] - i - 1) * stride_size / sizeof(float)],
				   stride_size);
		}

		// Set context input
		context->setInput(bufferName, channelName, img);
	}
}

template <typename TWrapper> void impl_st_reset_input(TWrapper &w)
{
	// Parse context id
	auto id(w.template GetParam<std::string>(0, "ctxt"));
	// Get context object
	auto context(host.GetContext(id));

	// Number of defined inputs
	long ninputs;

	// Get number of inputs (Mathematica)
	w.template ConditionalRun<OMWrapper<OMWT_MATHEMATICA>>([&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get input name
		auto inputName(w.template GetParam<std::string>(n + 1, "InputName"));

		// Parse name
		std::string bufferName;
		int channelName;
		impl_st_parse_input(inputName, bufferName, channelName);

		// Reset context input
		context->resetInput(bufferName, channelName);
	}
}

template <typename TWrapper> void impl_st_set_input_filter(TWrapper &w)
{
	// Parse context id
	auto id(w.template GetParam<std::string>(0, "ctxt"));
	// Get context object
	auto context(host.GetContext(id));

	// Number of defined inputs
	long ninputs;

	// Get number of inputs (Mathematica)
	w.template ConditionalRun<OMWrapper<OMWT_MATHEMATICA>>([&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get <name, image> tuple
		auto inputSpec(w.template GetParam<std::string, std::string>(n + 1, "InputSpec"));

		// Parse name
		std::string bufferName;
		int channelName;
		impl_st_parse_input(std::get<0>(inputSpec), bufferName, channelName);

		// Parse format
		auto filterMethod(std::get<1>(inputSpec));
		GLint minFilter;

		if (filterMethod.compare("Linear") == 0)
			minFilter = GL_LINEAR;
		else if (filterMethod.compare("Nearest") == 0)
			minFilter = GL_NEAREST;
		else if (filterMethod.compare("Mipmap") == 0)
			minFilter = GL_LINEAR_MIPMAP_LINEAR;
		else
		{
			std::stringstream ss;
			ss << "Invalid filter type '" << filterMethod << "' for " << std::get<0>(inputSpec);
			throw std::runtime_error(ss.str());
		}

		// Set context input filter
		context->setInputFilter(bufferName, channelName, minFilter);
	}
}

#endif /* _API_HPP_ */
