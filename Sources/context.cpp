#include "context.hpp"
#include "local.hpp"
#include "remote.hpp"

#ifndef _WIN32
#include <alloca.h>
#endif

#include <oglplus/dsa/ext/texture.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

StContext::StContext(const std::string &shaderId, int width, int height)
: shaderId(shaderId), config(), context(), frameCount(0), reloadInputConfig(false)
{
	initialize(shaderId, width, height);

	// Load the shader from the remote source
	loadRemote(shaderId, "fdnKWn", config);

	createContext(config);
}

StContext::StContext(const std::string &shaderId, const std::string &source, int width, int height)
: shaderId(shaderId), config(), context(), frameCount(0), reloadInputConfig(false)
{
	initialize(shaderId, width, height);

	// Load the shader from a locally created file
	loadLocal(shaderId, source, config);

	// Create the rendering context
	context = make_shared<shadertoy::RenderContext>(config);

	// Initialize it
	context->Initialize();
}

void StContext::performRender(GLFWwindow *window, int frameCount, int width, int height, float mouse[4], GLenum format)
{
	// Ensure we are working at the right size
	GLint depth = formatDepth(format);
	if (width != config.width || height != config.height || depth != currentImage.dims[2])
	{
		if (width != config.width || height != config.height)
		{
			config.width = width;
			config.height = height;
			context->AllocateTextures();
		}

		currentImage.dims[0] = height;
		currentImage.dims[1] = width;
		currentImage.dims[2] = depth;
		currentImage.data = make_shared<vector<float>>(depth * width * height);
	}

	// Reload inputs if necessary
	if (reloadInputConfig)
	{
		// Full reload
		context->GetTextureEngine().ClearState(false);
	}

	auto &state(context->GetState());

	// Poll events
	glfwPollEvents();

	// Update uniforms
	//  iTime and iFrame
	state.V<shadertoy::iTime>() = frameCount * (1.0 / config.targetFramerate);
	state.V<shadertoy::iFrame>() = frameCount;

	//  iDate
	boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
	state.V<shadertoy::iDate>() = oglplus::Vec4f(dt.date().year() - 1, dt.date().month(), dt.date().day(),
												 dt.time_of_day().total_nanoseconds() / 1e9f);

	//  iMouse
	for (int i = 0; i < 4; ++i)
		state.V<shadertoy::iMouse>()[i] = mouse[i];
	// End update uniforms

	// Render to texture
	context->Render();

	// Read the current texture
	GLint tex;
	context->DoReadCurrentFrame(tex);

	// Store it in a suitably sized array

	// float textures
	shared_ptr<vector<float>> texData = currentImage.data;
	glGetTextureImage(tex, 0, format, GL_FLOAT, sizeof(float) * width * height * depth, texData->data());

	// Vertical flip
	size_t stride_size = sizeof(float) * width * depth;
	float *stride = static_cast<float *>(alloca(stride_size));
	for (int i = 0; i < height / 2; ++i)
	{
		memcpy(stride, &(*texData)[i * stride_size / sizeof(float)], stride_size);
		memcpy(&(*texData)[i * stride_size / sizeof(float)],
			   &(*texData)[(height - i - 1) * stride_size / sizeof(float)], stride_size);
		memcpy(&(*texData)[(height - i - 1) * stride_size / sizeof(float)], stride, stride_size);
	}
}

void StContext::setInput(const string &buffer, int channel, StImage &image)
{
	auto &bufferOverrides(getBufferInputOverrides(buffer));

	// Override channel for this buffer
	auto it = bufferOverrides.find(channel);
	if (it == bufferOverrides.end())
	{
		// Change config type to the registered handler
		auto &input(config.bufferConfigs.find(buffer)->second.inputConfig[channel]);
		if (!input.enabled())
		{
			stringstream ss;
			ss << buffer << "." << channel << " input is not enabled";
			throw runtime_error(ss.str());
		}

		input.type = "data-" + input.type;

		// New override, reload config required
		bufferOverrides.insert(make_pair(channel, image));
		reloadInputConfig = true;
	}
	else
	{
		// Override already exists, do not reload config
		bufferOverrides[channel] = image;
	}
}

void StContext::resetInput(const string &buffer, int channel)
{
	auto &bufferOverrides(getBufferInputOverrides(buffer));

	// Override channel
	if (bufferOverrides.erase(channel) > 0)
	{
		auto &input(config.bufferConfigs.find(buffer)->second.inputConfig[channel]);
		input.type = string(input.type.begin() + (sizeof("data-") - 1), input.type.end());
		reloadInputConfig = true;
	}
}

void StContext::initialize(const std::string &shaderId, int width, int height)
{
	config.width = width;
	config.height = height;
	config.targetFramerate = 60.0;

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

oglplus::PixelDataFormat StContext::depthFormat(int depth)
{
	switch (depth)
	{
	case 4:
		return oglplus::PixelDataFormat::RGBA;
	case 3:
		return oglplus::PixelDataFormat::RGB;
	case 1:
	case 0: // no 3rd dimension
		return oglplus::PixelDataFormat::Red;
	default:
		throw runtime_error("Invalid depth");
	}
}

void StContext::createContext(shadertoy::ContextConfig &config)
{
	// Create the rendering context
	context = make_shared<shadertoy::RenderContext>(config);

	// Initialize it
	context->Initialize();

	// Register data texture handlers
	auto handler(shadertoy::InputHandler(bind(&StContext::DataTextureHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3, placeholders::_4)));

	context->GetTextureEngine().RegisterHandler("data-texture", handler);
	context->GetTextureEngine().RegisterHandler("data-buffer", handler);
}

map<int, StImage> &StContext::getBufferInputOverrides(const string &buffer)
{
	auto it = inputOverrides.find(buffer);
	if (it == inputOverrides.end())
	{
		const auto bufferConfigIt = config.bufferConfigs.find(buffer);
		if (bufferConfigIt == config.bufferConfigs.end())
		{
			stringstream ss;
			ss << buffer << " buffer not found";
			throw runtime_error(ss.str());
		}

		inputOverrides.insert(make_pair(buffer, map<int, StImage>()));
		it = inputOverrides.find(buffer);
	}

	return it->second;
}

shared_ptr<oglplus::Texture> StContext::DataTextureHandler(const shadertoy::InputConfig &inputConfig,
	bool &skipTextureOptions,
	bool &skipCache,
	bool &framebufferSized)
{
	skipCache = true;
	framebufferSized = false;

	// Get the texture object
	auto texPtr(getDataTexture(inputConfig.id));

	// Find the override texture
	string bufferName(inputConfig.id.begin(), inputConfig.id.begin() + inputConfig.id.find('.')),
		   inputName(inputConfig.id.begin() + inputConfig.id.find('.') + 1, inputConfig.id.end());
	int channel = atoi(inputName.c_str());

	StImage &img(getBufferInputOverrides(bufferName)[channel]);

	// Get the format of this image
	oglplus::PixelDataFormat fmt(depthFormat(img.dims[2]));

	// Load into OpenGL
	gl.DirectEXT(oglplus::TextureTarget::_2D, *texPtr)
		.Image2D(0, oglplus::PixelDataInternalFormat::RGBA32F,
			img.dims[1], img.dims[0], 0, fmt,
			oglplus::PixelDataType::Float, img.data->data());

	return texPtr;
}

shared_ptr<oglplus::Texture> StContext::getDataTexture(const string &inputId)
{
	auto it = textures.find(inputId);
	if (it == textures.end())
	{
		shared_ptr<oglplus::Texture> tex(make_shared<oglplus::Texture>());
		textures.insert(make_pair(inputId, tex));

		return tex;
	}

	return it->second;
}
