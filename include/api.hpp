#ifndef _API_HPP_
#define _API_HPP_

#include <array>
#include <exception>
#include <functional>
#include <sstream>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy.hpp>

#include <omw.hpp>

#include "context.hpp"
#include "host_host.hpp"

extern host_host host_mgr;

template<typename TWrapper>
void st_wrapper_internal(TWrapper &wrapper, std::function<void(TWrapper &)> fun)
{
	try
	{
		fun(wrapper);
	}
	catch (shadertoy::gl::shader_compilation_error &ex)
	{
		std::stringstream ss;
		ss << "Shader compilation error: " << ex.what() << std::endl << ex.log();
		wrapper.send_failure(ss.str(), "glerr");
	}
	catch (shadertoy::gl::program_link_error &ex)
	{
		std::stringstream ss;
		ss << "Program link error: " << ex.what() << std::endl << ex.log();
		wrapper.send_failure(ss.str(), "glerr");
	}
	catch (shadertoy::gl::opengl_error &ex)
	{
		std::stringstream ss;
		ss << "gl error: " << ex.what();
		wrapper.send_failure(ss.str(), "glerr");
	}
	catch (shadertoy::shadertoy_error &ex)
	{
		std::stringstream ss;
		ss << "Shadertoy error: " << ex.what();
		wrapper.send_failure(ss.str());
	}
	catch (std::runtime_error &ex)
	{
		wrapper.send_failure(ex.what());
	}
}

template <typename TWrapper, typename... ExtraArgs>
auto st_wrapper_exec(TWrapper &wrapper, std::function<void(TWrapper &)> fun, ExtraArgs... args)
{
	wrapper.check_initialization();

	return wrapper.run_function(args..., [&fun](TWrapper &wrapper) {
		st_wrapper_internal(wrapper, fun);
	});
}

template <typename TWrapper> void impl_st_set_renderer(TWrapper &w)
{
	// New renderer name
	std::string host(w.template get_param<std::string>(0, "Renderer"));

	// Add tcp:// if needed
	if (host.compare("local") != 0)
		if (std::strncmp(host.c_str(), "tcp", 3) != 0)
			host = std::string("tcp://") + host;

	// Add port number if needed
	if (std::strncmp(host.c_str(), "tcp", 3) == 0 && host.find(":", host.find(":") + 1) == std::string::npos)
		host += ":13710";

	// Set new renderer
	host_mgr.set_current(host);
}

template <typename TWrapper> void impl_st_compile(TWrapper &w)
{
	// Image buffer source
	std::string source(w.template get_param<std::string>(0, "code"));

	// Map of buffer sources
	std::vector<std::pair<std::string, std::string>> bufferSources;

	// Build buffer source map
	for (auto bufferSpec : w.template get_params<std::string, std::string>(1, "BufferSpec"))
	{
		// Lowercase buffer name
		std::string bufferName(std::get<0>(bufferSpec));
		std::transform(bufferName.begin(), bufferName.end(), bufferName.begin(), ::tolower);

		// Check for duplicates
		auto it = std::find_if(bufferSources.begin(), bufferSources.end(), [&bufferName](const auto &pair)
							   { return pair.first == bufferName; });
		if (it != bufferSources.end())
		{
			std::stringstream ss;
			ss << "Duplicate buffer name " << bufferName;
			throw std::runtime_error(ss.str());
		}

		bufferSources.emplace_back(std::move(bufferName), std::move(std::get<1>(bufferSpec)));
	}

	bufferSources.emplace_back(std::string("image"), std::move(source));

	std::string shaderId(host_mgr.current().create_local(bufferSources));
	w.write_result(shaderId);
}

template <typename TWrapper> void impl_st_reset(TWrapper &w)
{
	host_mgr.current().reset(w.template get_param<std::string>(0, "ctxt"));
}

template <typename TWrapper> void impl_st_render(TWrapper &w)
{
	auto id(w.template get_param<std::string>(0, "ctxt"));

	auto frameCount(w.template get_param<boost::optional<int>>(1, "Frame"));

	auto width(w.template get_param<boost::optional<int>>(2, "Width").get_value_or(640));
	auto height(w.template get_param<boost::optional<int>>(3, "Height").get_value_or(360));

	auto formatName(w.template get_param<boost::optional<std::string>>(4, "Format").get_value_or("RGBA"));
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

	auto mouse(w.template get_param<boost::optional<std::shared_ptr<omw::basic_array<float>>>>(5, "Mouse")
		.get_value_or(omw::vector_array<float>::make(4, 0.f)));

	std::array<float, 4> mouse_array;
	memcpy(mouse_array.data(), mouse->data(), sizeof(float) * (4 < mouse->size() ? 4 : mouse->size()));

	auto doFrameTiming(w.template get_param<boost::optional<bool>>(6, "FrameTiming").get_value_or(false));

	auto image(host_mgr.current().render(id, frameCount, width, height, mouse_array, format));
	auto image_result(omw::ref_matrix<float>::make(*image.data, image.dims));

	w.matrices_as_images(true);
	if (doFrameTiming)
	{
		w.write_result(image.frameTiming / 1e9, image_result);
	}
	else
	{
		w.write_result(image_result);
	}
}

bool impl_st_parse_input(std::string &inputSpecName, std::string &buffer, int &channel);

template <typename TWrapper> void impl_st_set_input(TWrapper &w)
{
	// Parse context id
	auto id(w.template get_param<std::string>(0, "ctxt"));
	// Get context object
	auto context(host_mgr.current().get_context(id));

	// Process all inputs
	for (auto inputSpec : w.template get_params<std::string, boost::variant<std::string, std::shared_ptr<omw::basic_matrix<float>>>>(1, "InputSpec"))
	{
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
			context->set_input(bufferName, channelName, boost::variant<std::string, std::shared_ptr<StImage>>(*inputBufferName));
		}
		else
		{
			// Get depth for tests
			auto imageValue(boost::get<std::shared_ptr<omw::basic_matrix<float>>>(inputValue));
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
			auto imgptr(std::make_shared<StImage>());
			auto &img(*imgptr);
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
			context->set_input(bufferName, channelName, imgptr);
		}
	}
}

template <typename TWrapper> void impl_st_reset_input(TWrapper &w)
{
	// Parse context id
	auto id(w.template get_param<std::string>(0, "ctxt"));
	// Get context object
	auto context(host_mgr.current().get_context(id));

	// Process all inputs
	for (auto inputName : w.template get_params<std::string>(1, "InputName"))
	{
		// Parse name
		std::string bufferName;
		int channelName(0);
		impl_st_parse_input(inputName, bufferName, channelName);

		// Reset context input
		context->reset_input(bufferName, channelName);
	}
}

template <typename TWrapper> void impl_st_set_input_filter(TWrapper &w)
{
	// Parse context id
	auto id(w.template get_param<std::string>(0, "ctxt"));
	// Get context object
	auto context(host_mgr.current().get_context(id));

	// Process all inputs
	for (auto inputSpec : w.template get_params<std::string, std::string>(1, "InputSpec"))
	{
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
		context->set_input_filter(bufferName, channelName, minFilter);
	}
}

#endif /* _API_HPP_ */
