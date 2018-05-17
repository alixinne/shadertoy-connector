#include <sstream>
#include <stdexcept>

#include "stc/core/getpid.h"

#include "stc/gl/context.hpp"
#include "stc/gl/host.hpp"
#include "stc/gl/remote.hpp"

using namespace stc;
using namespace stc::gl;

host::host() : basic_host(), st_window(nullptr), local_counter(0), st_contexts() {}

host::~host()
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
		free_remote();
		m_remoteInit = false;
	}
}

void st_glfwErrorCallback(int error, const char *description)
{
	std::stringstream ss;
	ss << "GLFW error: " << description;
	throw std::runtime_error(ss.str());
}

void host::allocate()
{
	init_remote();
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
			throw std::runtime_error("Could not create GLFW window");
		}
	}
	else
	{
		throw std::runtime_error("Could not initialize GLFW");
	}
}

core::image host::render(const std::string &id, boost::optional<int> frame, size_t width, size_t height,
					 const std::array<float, 4> &mouse, GLenum format)
{
	auto context(get_gl_context(id));

	// Default value for frame is the current frame count of the context
	if (!frame)
		frame = context->frame_count();

	// Render the next frame
	context->perform_render(*frame, width, height, mouse, format);

	// Return the image
	return context->current_image();
}

void host::reset(const std::string &id)
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

std::string host::create_local(const std::vector<std::pair<std::string, std::string>> &bufferSources)
{
	// Ensure we are in the right context
	glfwMakeContextCurrent(st_window);

	// Generate unique name
	std::stringstream name;
	name << "localshader-" << getpid() << "-" << local_counter++;
	std::string shaderId(name.str());

	// Create local context
	new_context(shaderId, bufferSources);

	return shaderId;
}

std::shared_ptr<core::basic_context> host::get_context(const std::string &id)
{
	return get_gl_context(id);
}

std::shared_ptr<context> host::get_gl_context(const std::string &id)
{
	// Ensure we are in the right context
	glfwMakeContextCurrent(st_window);

	auto it = st_contexts.find(id);
	if (it == st_contexts.end())
	{
		return new_context(id);
	}

	return it->second;
}
