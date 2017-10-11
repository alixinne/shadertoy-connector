#include <sstream>
#include <stdexcept>

#include "context.hpp"
#include "host.hpp"
#include "remote.hpp"

using namespace std;

Host::Host() : st_window(nullptr), local_counter(0), st_contexts() {}

Host::~Host()
{
	if (st_window)
	{
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

			if (glewInit() == GLEW_OK)
			{
				// ok
				return;
			}
			else
			{
				throw runtime_error("Could not initialize GLEW");
			}
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

StImage *Host::Render(const string &id, boost::optional<int> frame, int width, int height, float mouse[4])
{
	auto context(GetContext(id));

	// Default value for frame is the current frame count of the context
	if (!frame)
		frame = context->frameCount;

	// Render the next frame
	context->performRender(st_window, *frame, width, height, mouse);

	// Advance the frame counter
	context->frameCount = *frame + 1;

	// Return the image
	return &(context->currentImage);
}

string Host::CreateLocal(const string &source)
{
	stringstream name;
	name << "localshader-" << local_counter++;
	string shaderId(name.str());

	int width, height;
	glfwGetFramebufferSize(st_window, &width, &height);
	shared_ptr<StContext> ptr = make_shared<StContext>(shaderId, source, width, height);
	st_contexts.insert(make_pair(string(shaderId), ptr));
	return shaderId;
}

shared_ptr<StContext> Host::GetContext(const std::string &id)
{
	auto it = st_contexts.find(id);
	if (it == st_contexts.end())
	{
		int width, height;
		glfwGetFramebufferSize(st_window, &width, &height);
		shared_ptr<StContext> ptr = make_shared<StContext>(id, width, height);
		st_contexts.insert(make_pair(string(id), ptr));

		return ptr;
	}

	return it->second;
}
