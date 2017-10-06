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

#include "mathlink.h"

using namespace std;

extern "C" void st_render(const char *id);

GLFWwindow *st_window;

// Failure callback
void st_fail(const char *msgname, const char *arg)
{
	MLPutFunction(stdlink, "Message", 2);
	MLPutFunction(stdlink, "MessageName", 2);
	MLPutSymbol(stdlink, "RenderShadertoy");
	MLPutString(stdlink, msgname);
	MLPutString(stdlink, arg);
	MLPutSymbol(stdlink, "$Failed");
}

map<string, shared_ptr<StContext>> st_contexts;

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

void st_render(const char *id)
{
	try
	{
		st_get_context(id)->performRender(st_window);
	}
	catch (oglplus::ProgramBuildError &pbe)
	{
		stringstream ss;
		ss << "Program build error: " << pbe.what() << " [" << pbe.SourceFile() << ":"
		   << pbe.SourceLine() << "] " << pbe.Log();
		st_fail("glerr", ss.str().c_str());
	}
	catch (oglplus::Error &err)
	{
		stringstream ss;
		ss << "Error: " << err.what() << " [" << err.SourceFile() << ":" << err.SourceLine() << "] "
		   << err.Log();
		st_fail("glerr", ss.str().c_str());
	}
	catch (runtime_error &ex)
	{
		st_fail("err", ex.what());
	}
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
