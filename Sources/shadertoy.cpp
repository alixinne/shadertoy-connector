#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/all.hpp>

#include <shadertoy/Shadertoy.hpp>

#include "context.hpp"
#include "remote.hpp"
#include "wrapper.hpp"

#include "mathlink.h"

using namespace std;

extern "C" void st_render();
extern "C" char *st_compile(const char *source);

GLFWwindow *st_window;
int local_counter = 0;
map<string, shared_ptr<StContext>> st_contexts;

/**
 * Get or allocate a new remote context.
 *
 * @param  shaderId Context Id
 * @return          Pointer to the context
 */
shared_ptr<StContext> st_get_context(const string &shaderId)
{
	auto it = st_contexts.find(shaderId);
	if (it == st_contexts.end())
	{
		int width, height;
		glfwGetFramebufferSize(st_window, &width, &height);
		shared_ptr<StContext> ptr = make_shared<StContext>(shaderId, width, height);
		st_contexts.insert(make_pair(string(shaderId), ptr));

		return ptr;
	}

	return it->second;
}

/**
 * Instantiate a new unique local context.
 *
 * @param  source   Shader source code
 * @return          Name of the created context
 */
string st_new_context(const string &source)
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

char *st_compile(const char *source)
{
	return st_wrapper_exec(function<char*(void)>([&]() {
		return const_cast<char*>(st_new_context(source).c_str());
	}));
}

void st_render()
{
	const char *id_c;
	MLGetString(stdlink, &id_c);

	string id(id_c);
	MLReleaseString(stdlink, id_c);

	st_wrapper_exec(function<void(void)>([&]() {
		auto context(st_get_context(id));
		int frameCount;

		if (!MLGetInteger32(stdlink, &frameCount))
		{
			MLClearError(stdlink);

			const char *symbol;
			if (MLGetSymbol(stdlink, &symbol))
			{
				// Null is default value
				bool is_null = strcmp("Null", symbol) == 0;

				MLReleaseSymbol(stdlink, symbol);

				if (!is_null)
					throw runtime_error("Invalid value for Frame");
				else
					frameCount = context->frameCount;
			}
			else
			{
				MLClearError(stdlink);
				throw runtime_error("Could not get value for Frame");
			}
		}

		int width, height;
		MLGetInteger32(stdlink, &width);
		MLGetInteger32(stdlink, &height);

		float *mouseParam;
		int mouseParamSize;

		if (!MLGetReal32List(stdlink, &mouseParam, &mouseParamSize))
			throw runtime_error("Could not get value for Mouse");

		if (mouseParamSize != 2 && mouseParamSize != 4)
		{
			MLReleaseReal32List(stdlink, mouseParam, mouseParamSize);
			throw runtime_error("Mouse must be a list of size 2 or 4");
		}

		float mouse[4] = { 0.f, 0.f, 0.f, 0.f };
		memcpy(mouse, mouseParam, mouseParamSize * sizeof(float));
		MLReleaseReal32List(stdlink, mouseParam, mouseParamSize);

		// Render image
		context->performRender(st_window, frameCount, width, height, mouse);

		// Next frame
		context->frameCount = frameCount + 1;

		auto &image(context->currentImage);
		MLPutFunction(stdlink, "Image", 1);
		MLPutReal32Array(stdlink, image.data->data(), &image.dims[0], NULL, 3);
	}));
}

enum st_ERRORS
{
	st_GLFW_INIT_ERROR,
	st_GLFW_WINDOW_ERROR,
	st_GLEW_ERROR
};

// GLFW error callback
void st_glfwErrorCallback(int error, const char *description)
{
	st_fail("glfwerr", description);
}

// Shadertoy module entrypoint
int st_main(int argc, char *argv[])
{
	int code = 0;

	initRemote();

	// Error callback
	glfwSetErrorCallback(st_glfwErrorCallback);

	// Initialize GLFW rendering context
	if (glfwInit())
	{
		// Create the rendering context and make it current
		glfwWindowHint(GLFW_VISIBLE, 0);

		// Initialize window
		st_window = glfwCreateWindow(640, 360, "Mathematica Shadertoy Renderer", nullptr, nullptr);

		if (st_window)
		{
			// Set current
			glfwMakeContextCurrent(st_window);

			// Load GLEW
			if (glewInit() == GLEW_OK)
			{
				// Run ML module
				code = MLMain(argc, argv);
			}
			else
			{
				code = st_GLEW_ERROR;
			}

			// Clean up
			glfwDestroyWindow(st_window);
		}
		else
		{
			code = st_GLFW_WINDOW_ERROR;
		}

		// Free up resources
		glfwTerminate();
	}
	else
	{
		code = st_GLFW_INIT_ERROR;
	}

	freeRemote();

	return code;
}

// Entry point for MathLink

#if WINDOWS_MATHLINK

#if __BORLANDC__
#pragma argsused
#endif

int PASCAL WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
	char buff[512];
	char FAR *buff_start = buff;
	char FAR *argv[32];
	char FAR *FAR *argv_end = argv + 32;

	hinstPrevious = hinstPrevious; /* suppress warning */

	if (!MLInitializeIcon(hinstCurrent, nCmdShow))
		return 1;
	MLScanString(argv, &argv_end, &lpszCmdLine, &buff_start);
	return st_main((int)(argv_end - argv), argv);
}

#else

int main(int argc, char *argv[]) { return st_main(argc, argv); }

#endif
