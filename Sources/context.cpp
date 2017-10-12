#include "context.hpp"
#include "local.hpp"
#include "remote.hpp"

#include <alloca.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

StContext::StContext(const std::string &shaderId, int width, int height)
: shaderId(shaderId), config(), context(), frameCount(0)
{
	initialize(shaderId, width, height);

	// Load the shader from the remote source
	loadRemote(shaderId, "fdnKWn", config);

	// Create the rendering context
	context = make_shared<shadertoy::RenderContext>(config);

	// Initialize it
	context->Initialize();
}

StContext::StContext(const std::string &shaderId, const std::string &source, int width, int height)
: shaderId(shaderId), config(), context(), frameCount(0)
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
