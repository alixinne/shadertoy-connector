#include "context.hpp"
#include "remote.hpp"

#include <alloca.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "mathlink.h"

using namespace std;

StContext::StContext(const std::string &shaderId, int width, int height)
: shaderId(shaderId), config(), context(), frameCount(0)
{
	config.width = width;
	config.height = height;

	// Load the shader from the remote source
	loadRemote(shaderId, "fdnKWn", config);

	// Create the rendering context
	context = make_shared<shadertoy::RenderContext>(config);

	// Initialize it
	context->Initialize();
}

void StContext::performRender(GLFWwindow *window)
{
	// Ensure we are working at the right size
	int win_width, win_height;
	glfwGetFramebufferSize(window, &win_width, &win_height);
	if (win_width != config.width || win_height != config.height)
	{
		config.width = win_width;
		config.height = win_height;
		context->AllocateTextures();
	}

	auto state(context->GetState());

	// Poll events
	glfwPollEvents();

	// Update uniforms
	//  iTime and iFrame
	state.V<shadertoy::iTime>() += 1.0 / config.targetFramerate;
	state.V<shadertoy::iFrame>() = frameCount;

	//  iDate
	boost::posix_time::ptime dt = boost::posix_time::microsec_clock::local_time();
	state.V<shadertoy::iDate>() = oglplus::Vec4f(dt.date().year() - 1, dt.date().month(), dt.date().day(),
												 dt.time_of_day().total_nanoseconds() / 1e9f);

	//  iMouse
	int btnstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (btnstate == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		state.V<shadertoy::iMouse>()[0] = state.V<shadertoy::iMouse>()[2] = xpos;
		state.V<shadertoy::iMouse>()[1] = state.V<shadertoy::iMouse>()[3] = config.height - ypos;
	}
	else
	{
		state.V<shadertoy::iMouse>()[2] = state.V<shadertoy::iMouse>()[3] = 0.f;
	}
	// End update uniforms

	// Render to texture
	context->Render();

	// Read the current texture
	GLint tex;
	context->DoReadCurrentFrame(tex);

	// Store it in a suitably sized array
	GLint width, height, depth;
	glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_WIDTH, &width);
	glGetTextureLevelParameteriv(tex, 0, GL_TEXTURE_HEIGHT, &height);
	depth = 3; // RGB output

	// float textures
	float *texData = new float[width * height * depth];
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, sizeof(float) * width * height * depth, texData);

	// Vertical flip
	size_t stride_size = sizeof(float) * width * depth;
	float *stride = static_cast<float *>(alloca(stride_size));
	for (int i = 0; i < height / 2; ++i)
	{
		memcpy(stride, &texData[i * stride_size / sizeof(float)], stride_size);
		memcpy(&texData[i * stride_size / sizeof(float)],
			   &texData[(height - i - 1) * stride_size / sizeof(float)], stride_size);
		memcpy(&texData[(height - i - 1) * stride_size / sizeof(float)], stride, stride_size);
	}

	// Send this to Mathematica
	int dims[] = { height, width, depth };
	MLPutFunction(stdlink, "Image", 1);
	MLPutReal32Array(stdlink, texData, &dims[0], NULL, 3);

	delete[] texData;

	// Update time and framecount
	frameCount++;
}
