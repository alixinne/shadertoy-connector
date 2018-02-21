#include <sstream>
#include <stdexcept>

#include "getpid.h"

#include "context.hpp"
#include "host.hpp"
#include "remote.hpp"

using namespace std;

Host::Host() : st_window(nullptr), local_counter(0), st_contexts() {}

Host::~Host()
{
	if (st_window)
	{
		// We first need to delete all the StContexts or else GL destructors will
		// fail because there is no current OpenGL context
		glfwMakeContextCurrent(st_window);

		// Destroy contexts
		st_contexts.clear();

		// Destroy window
		glfwDestroyWindow(st_window);
		st_window = nullptr;
	}

	if (m_glfwInit)
	{
		glfwTerminate();
		m_glfwInit = false;
	}

	if (m_remoteInit)
	{
		freeRemote();
		m_remoteInit = false;
	}
}

void st_glfwErrorCallback(int error, const char *description)
{
	stringstream ss;
	ss << "GLFW error: " << description;
	throw runtime_error(ss.str());
}

void Host::Allocate()
{
	initRemote();
	m_remoteInit = true;

	// Set callback
	glfwSetErrorCallback(st_glfwErrorCallback);

	// Initialize GLFW
	if (glfwInit())
	{
		m_glfwInit = true;

		// Create the rendering context
		glfwWindowHint(GLFW_VISIBLE, 0);
		st_window = glfwCreateWindow(640, 360, "Shadertoy Connector Renderer", nullptr, nullptr);

		if (st_window)
		{
			glfwMakeContextCurrent(st_window);

			// ok
			return;
		}
		else
		{
			throw runtime_error("Could not create GLFW window");
		}
	}
	else
	{
		throw runtime_error("Could not initialize GLFW");
	}
}

StImage *Host::Render(const string &id, boost::optional<int> frame, int width, int height,
					  const float mouse[4], GLenum format)
{
	auto context(GetContext(id));

	// Default value for frame is the current frame count of the context
	if (!frame)
		frame = context->frameCount;

	// Render the next frame
	context->performRender(st_window, *frame, width, height, mouse, format);

	// Advance the frame counter
	context->frameCount = *frame + 1;

	// Return the image
	return &(context->currentImage);
}

void Host::Reset(const string &id)
{
	// Ensure we are in the right context
	glfwMakeContextCurrent(st_window);

	const char local[] = "localshader-";
	if (strncmp(id.c_str(), local, sizeof(local) - 1) == 0)
	{
		throw std::runtime_error("Cannot reset a local context.");
	}

	st_contexts.erase(id);
}

string Host::CreateLocal(const string &source)
{
	// Ensure we are in the right context
	glfwMakeContextCurrent(st_window);

	// Generate unique name
	stringstream name;
	name << "localshader-" << getpid() << "-" << local_counter++;
	string shaderId(name.str());

	// Create local context
	NewContext(shaderId, source);

	return shaderId;
}

shared_ptr<StContext> Host::GetContext(const std::string &id)
{
	// Ensure we are in the right context
	glfwMakeContextCurrent(st_window);

	auto it = st_contexts.find(id);
	if (it == st_contexts.end())
	{
		return NewContext(id);
	}

	return it->second;
}
