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

template<typename TWrapper>
void st_wrapper_internal(TWrapper &wrapper, std::function<void(TWrapper &)> fun)
{
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
}

template <typename TWrapper, typename... ExtraArgs>
auto st_wrapper_exec(TWrapper &wrapper, std::function<void(TWrapper &)> fun, ExtraArgs... args)
{
	wrapper.CheckInitialization();

	return wrapper.RunFunction(args..., [&fun](TWrapper &wrapper) {
		st_wrapper_internal(wrapper, fun);
	});
}

std::string mathematica_unescape(const std::string &source);

template <typename TWrapper> void impl_st_compile(TWrapper &w)
{
	std::string source(w.template GetParam<std::string>(0, "code"));

	OM_MATHEMATICA(w, [&source]() {
		source = mathematica_unescape(source);
	});

	std::string shaderId(host.CreateLocal(source));

	OM_RESULT_MATHEMATICA(w, [&]() { MLPutString(w.link, shaderId.c_str()); });

	OM_RESULT_OCTAVE(w, [&]() { w.Result().append(shaderId); });
}

template <typename TWrapper> void impl_st_reset(TWrapper &w)
{
	host.Reset(w.template GetParam<std::string>(0, "ctxt"));
}

template <typename TWrapper> void impl_st_render(TWrapper &w)
{
	auto id(w.template GetParam<std::string>(0, "ctxt"));

	auto frameCount(w.template GetParam<boost::optional<int>>(1, "Frame"));

	auto width(w.template GetParam<boost::optional<int>>(2, "Width").get_value_or(640));
	auto height(w.template GetParam<boost::optional<int>>(3, "Height").get_value_or(360));

	auto formatName(w.template GetParam<boost::optional<std::string>>(4, "Format").get_value_or("RGBA"));
	std::transform(formatName.begin(), formatName.end(),
		formatName.begin(), ::tolower);
	GLenum format;

	if (formatName.compare("rgba") == 0)
		format = GL_RGBA;
	else if (formatName.compare("rgb") == 0)
		format = GL_RGB;
	else if (formatName.compare("luminance") == 0)
		format = GL_LUMINANCE;
	else
		throw std::runtime_error("Invalid Format parameter");

	auto mouse(w.template GetParam<boost::optional<std::shared_ptr<OMArray<float>>>>(5, "Mouse")
		.get_value_or(OMArray<float>::from_vector(4, 0.f)));

	auto doFrameTiming(w.template GetParam<boost::optional<bool>>(6, "FrameTiming").get_value_or(false));

	auto image(host.Render(id, frameCount, width, height, mouse->data(), format));

	OM_RESULT_MATHEMATICA(w, [&]() {
		if (doFrameTiming)
		{
			MLPutFunction(w.link, "List", 2);
			MLPutReal64(w.link, image->frameTiming / 1e9);
		}

		MLPutFunction(w.link, "Image", 1);
		MLPutReal32Array(w.link, image->data->data(), &image->dims[0], NULL, 3);
	});

	OM_RESULT_OCTAVE(w, [&]() {
		auto &res(w.Result());

		// Return the image
		auto dims(dim_vector(image->dims[0], image->dims[1], image->dims[2]));
		NDArray data(dims);

		// Need to copy from float* to double*
		for (int i = 0; i < image->dims[0]; ++i)
			for (int j = 0; j < image->dims[1]; ++j)
				for (int k = 0; k < image->dims[2]; ++k)
				{
					size_t idx = (i * image->dims[1] + j) * image->dims[2] + k;
					data(i, j, k) = static_cast<double>((*image->data)[idx]);
				}

		if (doFrameTiming)
		{
			res.append(image->frameTiming / 1e9);
		}

		res.append(data);
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
	OM_MATHEMATICA(w, [&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Get number of inputs (Octave)
	OM_OCTAVE(w, [&]() {
		ninputs = (w.Args().length() - 1) / 2;
	});

	// Get size of fetched tuples
	int tupleSize = 1;
	OM_OCTAVE(w, [&]() {
		tupleSize = 2;
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get <name, image> tuple
		auto inputSpec(w.template GetParam<std::tuple<std::string, boost::variant<std::string, std::shared_ptr<OMMatrix<float>>>>>(tupleSize * n + 1, "InputSpec"));

		// Parse name
		std::string bufferName;
		int channelName(0);
		impl_st_parse_input(std::get<0>(inputSpec), bufferName, channelName);

		// Get the input value
		auto inputValue(std::get<1>(inputSpec));

		if (std::string *inputBufferName = boost::get<std::string>(&inputValue))
		{
			std::transform(inputBufferName->begin(), inputBufferName->end(),
				inputBufferName->begin(), ::tolower);
			context->setInput(bufferName, channelName, boost::variant<std::string, StImage>(*inputBufferName));
		}
		else
		{
			// Get depth for tests
			auto imageValue(boost::get<std::shared_ptr<OMMatrix<float>>>(inputValue));
			int d = imageValue->depth();

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
			img.dims[0] = imageValue->dims()[0];
			img.dims[1] = imageValue->dims()[1];
			if (d == 3)
				img.dims[2] = imageValue->dims()[2];
			else
				img.dims[2] = 1;
			img.data = std::make_shared<std::vector<float>>(img.dims[0] * img.dims[1] * img.dims[2]);

			// Copy data, vflip
			size_t stride_size = sizeof(float) * img.dims[1] * img.dims[2];
			for (int i = 0; i < img.dims[0]; ++i)
			{
				memcpy(&img.data->data()[i * stride_size / sizeof(float)],
					   &imageValue->data()[(img.dims[0] - i - 1) * stride_size / sizeof(float)],
					   stride_size);
			}

			// Set context input
			context->setInput(bufferName, channelName, img);
		}
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
	OM_MATHEMATICA(w, [&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Get number of inputs (Octave)
	OM_OCTAVE(w, [&]() {
		ninputs = (w.Args().length() - 1) / 2;
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get input name
		auto inputName(w.template GetParam<std::string>(n + 1, "InputName"));

		// Parse name
		std::string bufferName;
		int channelName(0);
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
	OM_MATHEMATICA(w, [&]() {
		if (!MLCheckFunction(w.link, "List", &ninputs))
		{
			MLClearError(w.link);
			throw std::runtime_error("Invalid input specification");
		}
	});

	// Get number of inputs (Octave)
	OM_OCTAVE(w, [&]() {
		ninputs = (w.Args().length() - 1) / 2;
	});

	// Get size of fetched tuples
	int tupleSize = 1;
	OM_OCTAVE(w, [&]() {
		tupleSize = 2;
	});

	// Process all inputs
	for (long n = 0; n < ninputs; ++n)
	{
		// Get <name, image> tuple
		auto inputSpec(w.template GetParam<std::tuple<std::string, std::string>>(tupleSize * n + 1, "InputSpec"));

		// Parse name
		std::string bufferName;
		int channelName(0);
		impl_st_parse_input(std::get<0>(inputSpec), bufferName, channelName);

		// Parse format
		auto filterMethod(std::get<1>(inputSpec));
		std::transform(filterMethod.begin(), filterMethod.end(),
			filterMethod.begin(), ::tolower);
		GLint minFilter;

		if (filterMethod.compare("linear") == 0)
			minFilter = GL_LINEAR;
		else if (filterMethod.compare("nearest") == 0)
			minFilter = GL_NEAREST;
		else if (filterMethod.compare("mipmap") == 0)
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
