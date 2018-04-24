#include "context.hpp"
#include "local.hpp"
#include "remote.hpp"

#ifndef _WIN32
#include <alloca.h>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

StContext::StContext(const std::string &shaderId, size_t width, size_t height)
: shaderId(shaderId), render_size(width, height), context(), chain(), frameCount(0)
{
	initialize(shaderId, width, height);

	// Load the shader from the remote source
	loadRemote(shaderId, "fdnKWn", context, chain, render_size);

	createContext();
}

StContext::StContext(const std::string &shaderId,
					 const std::vector<std::pair<std::string, std::string>> &bufferSources,
					 size_t width, size_t height)
: shaderId(shaderId), render_size(width, height), context(), chain(), frameCount(0)
{
	initialize(shaderId, width, height);

	// Load the shader from a locally created file
	loadLocal(shaderId, bufferSources, context, chain, render_size);

	createContext();
}

void StContext::performRender(GLFWwindow *window, int frameCount, size_t width, size_t height,
							  const float mouse[4], GLenum format)
{
	// Ensure we are working at the right size
	GLint depth = formatDepth(format);
	if (width != render_size.width || height != render_size.height || depth != currentImage.dims[2])
	{
		if (width != render_size.width || height != render_size.height)
		{
			render_size.width = width;
			render_size.height = height;
			context.allocate_textures(chain);
		}

		currentImage.dims[0] = height;
		currentImage.dims[1] = width;
		currentImage.dims[2] = depth;
		currentImage.data = make_shared<vector<float>>(depth * width * height);
	}

	auto &state(context.state());

	// Poll events
	glfwPollEvents();

	// Update uniforms
	//  iTime and iFrame
	state.get<shadertoy::iTime>() = frameCount * (1.0 / state.get<shadertoy::iFrameRate>());
	state.get<shadertoy::iFrame>() = frameCount;

	//  iDate
	boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
	state.get<shadertoy::iDate>() = glm::vec4(dt.date().year() - 1, dt.date().month(), dt.date().day(),
											  dt.time_of_day().total_nanoseconds() / 1e9f);

	//  iMouse
	for (int i = 0; i < 4; ++i)
		state.get<shadertoy::iMouse>()[i] = mouse[i];
	// End update uniforms

	// Render to texture
	context.render(chain);

	// Read the current texture
	auto tex(chain.current()->output());

	// Store it in a suitably sized array

	// float textures
	shared_ptr<vector<float>> texData = currentImage.data;
	tex->get_image(0, format, GL_FLOAT, sizeof(float) * width * height * depth, texData->data());

	// Vertical flip
	size_t stride_size = sizeof(float) * width * depth;
	float *stride = static_cast<float *>(alloca(stride_size));
	for (size_t i = 0; i < height / 2; ++i)
	{
		memcpy(stride, &(*texData)[i * stride_size / sizeof(float)], stride_size);
		memcpy(&(*texData)[i * stride_size / sizeof(float)],
			   &(*texData)[(height - i - 1) * stride_size / sizeof(float)], stride_size);
		memcpy(&(*texData)[(height - i - 1) * stride_size / sizeof(float)], stride, stride_size);
	}

	currentImage.frameTiming = std::static_pointer_cast<shadertoy::members::buffer_member>(chain.current())->buffer()->elapsed_time();
}

void StContext::setInput(const std::string &buffer, size_t channel,
						 const boost::variant<std::string, std::shared_ptr<StImage>> &data)
{
	// Get a reference to the buffer
	auto toy_buffer(getBuffer(buffer));

	// Reference to the target input
	if (channel > toy_buffer->inputs().size())
	{
		std::stringstream ss;
		ss << "Channel " << channel << " is not available on " << buffer;
		throw std::runtime_error(ss.str());
	}

	auto &input(toy_buffer->inputs()[channel]);

	// The input that will be overridden by this call
	auto overriden_input = input.input();

	if (auto ov_input = std::dynamic_pointer_cast<override_input>(input.input()))
	{
		// Input is already overridden, so replace it
		overriden_input = ov_input->overriden_input();
	} // else, use "input" as the overridden input

	if (const auto img = boost::get<const std::shared_ptr<StImage>>(&data))
	{
		// The override is an image input
		input.input(std::make_shared<override_input>(overriden_input, *img));
	}
	else
	{
		auto target_name(boost::get<const std::string>(data));

		// The override is a buffer name
		auto member_source(chain.find_if<shadertoy::members::buffer_member>(
		[&target_name](const auto &member) { return member->buffer()->id() == target_name; }));

		if (!member_source)
		{
			std::stringstream ss;
			ss << "Buffer " << target_name << " was not found to override " << buffer << "." << channel;
			throw std::runtime_error(ss.str());
		}

		input.input(std::make_shared<override_input>(overriden_input,
													 std::make_shared<shadertoy::inputs::buffer_input>(member_source)));
	}
}

void StContext::setInputFilter(const string &buffer, size_t channel, GLint minFilter)
{
	// Get a reference to the buffer
	auto toy_buffer(getBuffer(buffer));

	// Reference to the target input
	if (channel > toy_buffer->inputs().size())
	{
		std::stringstream ss;
		ss << "Channel " << channel << " is not available on " << buffer;
		throw std::runtime_error(ss.str());
	}

	auto input(toy_buffer->inputs()[channel].input());

	if (!input)
	{
		std::stringstream ss;
		ss << "Input " << buffer << "." << channel
		   << " has not been yet initialized, so its filter cannot be set";
		throw std::runtime_error(ss.str());
	}

	input->min_filter(minFilter);
	input->mag_filter(minFilter == GL_NEAREST ? GL_NEAREST : GL_LINEAR);
}

void StContext::resetInput(const string &buffer, size_t channel)
{
	// Get a reference to the buffer
	auto toy_buffer(getBuffer(buffer));

	// Reference to the target input
	if (channel > toy_buffer->inputs().size())
	{
		std::stringstream ss;
		ss << "Channel " << channel << " is not available on " << buffer;
		throw std::runtime_error(ss.str());
	}

	auto &input(toy_buffer->inputs()[channel]);

	if (auto ov_input = std::dynamic_pointer_cast<override_input>(input.input()))
	{
		input.input(ov_input->overriden_input());
	}
}

void StContext::initialize(const std::string &shaderId, size_t width, size_t height)
{
	context.state().get<shadertoy::iTimeDelta>() = 1.0 / 60.0;
	context.state().get<shadertoy::iFrameRate>() = 60.0;

	currentImage.dims[0] = height;
	currentImage.dims[1] = width;
	currentImage.dims[2] = formatDepth(GL_RGB);
	currentImage.data = make_shared<vector<float>>(currentImage.dims[2] * width * height);
}

int StContext::formatDepth(GLenum format)
{
	switch (format)
	{
	case GL_RGBA:
		return 4;
	case GL_RGB:
		return 3;
	case GL_LUMINANCE:
		return 1;
	default:
		throw runtime_error("Invalid format");
	}
}

GLint StContext::depthFormat(int depth)
{
	switch (depth)
	{
	case 4:
		return GL_RGBA;
	case 3:
		return GL_RGB;
	case 1:
	case 0: // no 3rd dimension
		return GL_RED;
	default:
		throw runtime_error("Invalid depth");
	}
}

void StContext::createContext()
{
	// Initialize the swap chain
	context.init(chain);
}

StContext::override_input::override_input(std::shared_ptr<shadertoy::inputs::basic_input> overriden_input,
										  std::shared_ptr<StImage> data_buffer)
: data_buffer_(data_buffer), texture_(), member_input_(), overriden_input_(overriden_input)
{
	assert(data_buffer);
}

StContext::override_input::override_input(std::shared_ptr<shadertoy::inputs::basic_input> overriden_input,
										  std::shared_ptr<shadertoy::inputs::buffer_input> member_input)
: data_buffer_(), texture_(), member_input_(member_input), overriden_input_(overriden_input)
{
	assert(member_input);
}

void StContext::override_input::load_input()
{
	if (data_buffer_)
	{
		// Data buffer mode
		//  Create texture object if needed
		if (!texture_)
			texture_ = std::make_shared<shadertoy::gl::texture>(GL_TEXTURE_2D);

		// Get the format of this image
		GLint fmt(depthFormat(data_buffer_->dims[2]));

		//  Load into OpenGL
		texture_->image_2d(GL_TEXTURE_2D, 0, GL_RGBA32F, data_buffer_->dims[1],
						   data_buffer_->dims[0], 0, fmt, GL_FLOAT, data_buffer_->data->data());

		texture_->generate_mipmap();
	}
	else
	{
		member_input_->load();
	}
}

void StContext::override_input::reset_input()
{
	if (data_buffer_)
	{
	}
	else
	{
		member_input_->reset();
	}
}

std::shared_ptr<shadertoy::gl::texture> StContext::override_input::use_input()
{
	if (data_buffer_)
	{
		return texture_;
	}
	else
	{
		return member_input_->use();
	}
}

std::shared_ptr<shadertoy::buffers::toy_buffer> StContext::getBuffer(const std::string &name)
{
	// Find the target buffer by its name
	auto buffer_member(chain.find_if<shadertoy::members::buffer_member>([&name](const auto &member) {
																		return member->buffer()->id() == name;
																		}));

	if (!buffer_member)
	{
		std::stringstream ss;
		ss << "Buffer " << std::quoted(name) << " was not found in " << shaderId;
		throw std::runtime_error(ss.str());
	}

	// Get the actual buffer object
	return std::static_pointer_cast<shadertoy::buffers::toy_buffer>(buffer_member->buffer());
}
