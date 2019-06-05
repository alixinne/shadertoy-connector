#include "stc/gl/context.hpp"
#include "stc/gl/local.hpp"
#include "stc/gl/remote.hpp"

#ifndef _WIN32
#include <alloca.h>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace stc;
using namespace stc::gl;

context::context(const std::string &shaderId, size_t width, size_t height)
: core::basic_context(shaderId), render_size_(width, height), context_(), chain_(), frame_count_(0)
{
	initialize(shaderId, width, height);

	// Load the shader from the remote source
	load_remote(shaderId, "fdnKWn", context_, chain_, render_size_);

	create_context();
}

context::context(const std::string &shaderId,
				 const std::vector<std::pair<std::string, std::string>> &bufferSources,
				 size_t width, size_t height)
: core::basic_context(shaderId), render_size_(width, height), context_(), chain_(), frame_count_(0)
{
	initialize(shaderId, width, height);

	// Load the shader from a locally created file
	load_local(shaderId, bufferSources, context_, chain_, render_size_);

	create_context();
}

void context::perform_render(int frameCount, size_t width, size_t height,
							 const std::array<float, 4> &mouse, GLenum format)
{
	// Ensure we are working at the right size
	GLint depth = format_depth(format);
	if (width != render_size_.width || height != render_size_.height || depth != static_cast<GLint>(current_image_.dims[2]))
	{
		if (width != render_size_.width || height != render_size_.height)
		{
			render_size_.width = width;
			render_size_.height = height;
			context_.allocate_textures(chain_);
		}

		current_image_.dims[0] = height;
		current_image_.dims[1] = width;
		current_image_.dims[2] = depth;
		current_image_.alloc();
	}

	// Poll events
	glfwPollEvents();

	// Update uniforms
	//  iFrameRate, iTime, iFrame
	chain_.set_uniform("iFrameRate", 60.0f);
	chain_.set_uniform("iTimeDelta", 1.0f / 60.0f);
	chain_.set_uniform("iTime", frameCount * (1.0f / 60.0f));
	chain_.set_uniform("iFrame", frameCount);

	//  iDate
	boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
	chain_.set_uniform("iDate", glm::vec4(dt.date().year() - 1, dt.date().month(), dt.date().day(),
										  dt.time_of_day().total_nanoseconds() / 1e9f));

	//  iMouse
	chain_.set_uniform("iMouse", glm::vec4(mouse[0], mouse[1], mouse[2], mouse[3]));
	// End update uniforms

	// Render to texture
	context_.render(chain_);

	// Read the current texture
	auto tex(chain_.current()->output().front());

	// Store it in a suitably sized array

	// float textures
	std::shared_ptr<std::vector<float>> texData = current_image_.data;
	std::get<1>(tex)->get_image(0, format, GL_FLOAT, sizeof(float) * width * height * depth, texData->data());

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

	current_image_.frame_timing = std::static_pointer_cast<shadertoy::members::buffer_member>(chain_.current())->buffer()->elapsed_time();

	// Advance the frame counter
	frame_count_ = frameCount + 1;
}

void context::set_input(const std::string &buffer, size_t channel,
						const boost::variant<std::string, std::shared_ptr<core::image>> &data)
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
	std::shared_ptr<override_input> ov_input;

	if (!(ov_input = std::dynamic_pointer_cast<override_input>(input.input())))
	{
		// The input has not been overriden yet, so we replace it
		input.input(ov_input = std::make_shared<override_input>(input.input()));
	} // else, we will modify ov_input state

	if (const auto img = boost::get<const std::shared_ptr<core::image>>(&data))
	{
		// The override is an image input
		ov_input->set(*img);
	}
	else
	{
		auto target_name(boost::get<const std::string>(data));

		// The override is a buffer name
		auto member_source(chain_.find_if<shadertoy::members::buffer_member>(
		[&target_name](const auto &member) { return member->buffer()->id() == target_name; }));

		if (!member_source)
		{
			std::stringstream ss;
			ss << "Buffer " << target_name << " was not found to override " << buffer << "." << channel;
			throw std::runtime_error(ss.str());
		}

		ov_input->set(std::make_shared<shadertoy::inputs::buffer_input>(member_source));
	}

	ov_input->reset();
}

void context::set_input_filter(const std::string &buffer, size_t channel, GLint minFilter)
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

void context::reset_input(const std::string &buffer, size_t channel)
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

void context::initialize(const std::string &shaderId, size_t width, size_t height)
{
	current_image_.dims[0] = height;
	current_image_.dims[1] = width;
	current_image_.dims[2] = format_depth(GL_RGB);
	current_image_.alloc();
}

int context::format_depth(GLenum format)
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
		throw std::runtime_error("Invalid format");
	}
}

GLint context::depth_format(int depth)
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
		throw std::runtime_error("Invalid depth");
	}
}

void context::create_context()
{
	// Initialize the swap chain
	context_.init(chain_);
}

context::override_input::override_input(std::shared_ptr<shadertoy::inputs::basic_input> overriden_input)
: data_buffer_(), texture_(), member_input_(), overriden_input_(overriden_input)
{
}

void context::override_input::set(std::shared_ptr<core::image> data_buffer)
{
	assert(data_buffer);

	member_input_.reset();

	data_buffer_ = data_buffer;
}

void context::override_input::set(std::shared_ptr<shadertoy::inputs::buffer_input> member_input)
{
	assert(member_input);

	data_buffer_.reset();
	texture_.reset();

	member_input_ = member_input;
}

void context::override_input::load_input()
{
	if (data_buffer_)
	{
		// Data buffer mode
		//  Create texture object if needed
		if (!texture_)
			texture_ = std::make_shared<shadertoy::gl::texture>(GL_TEXTURE_2D);

		// Get the format of this image
		GLint fmt(depth_format(data_buffer_->dims[2]));

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

void context::override_input::reset_input()
{
	if (data_buffer_)
	{
	}
	else
	{
		member_input_->reset();
	}
}

shadertoy::gl::texture *context::override_input::use_input()
{
	if (data_buffer_)
	{
		return texture_.get();
	}
	else
	{
		return member_input_->use();
	}
}

std::shared_ptr<shadertoy::buffers::toy_buffer> context::getBuffer(const std::string &name)
{
	// Find the target buffer by its name
	auto buffer_member(chain_.find_if<shadertoy::members::buffer_member>([&name](const auto &member) {
																		return member->buffer()->id() == name;
																		}));

	if (!buffer_member)
	{
		std::stringstream ss;
		ss << "Buffer " << std::quoted(name) << " was not found in " << id();
		throw std::runtime_error(ss.str());
	}

	// Get the actual buffer object
	return std::static_pointer_cast<shadertoy::buffers::toy_buffer>(buffer_member->buffer());
}
