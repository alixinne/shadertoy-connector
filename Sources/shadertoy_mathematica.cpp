#include <functional>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/all.hpp>

#include "context.hpp"
#include "host.hpp"

#include "mathlink.h"

using namespace std;

extern "C" void st_render();
extern "C" char *st_compile(const char *source);

// Render context host
Host host;

// Failure callback
void st_fail(const char *msgname, const char *arg)
{
	MLPutFunction(stdlink, "CompoundExpression", 2);
	MLPutFunction(stdlink, "Message", 2);
	MLPutFunction(stdlink, "MessageName", 2);
	MLPutSymbol(stdlink, "Shadertoy");
	MLPutString(stdlink, msgname);
	MLPutString(stdlink, arg);
	MLPutSymbol(stdlink, "$Failed");
}

template <typename TRet> TRet st_wrapper_exec(function<TRet(void)> &&fun)
{
	try
	{
		return fun();
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

	return TRet();
}

template <> void st_wrapper_exec(function<void(void)> &&fun)
{
	st_wrapper_exec<bool>(function<bool(void)>([&fun]() {
		fun();
		return true;
	}));
}

char *st_compile(const char *source)
{
	return st_wrapper_exec(
	function<char *(void)>([&]() { return const_cast<char *>(host.CreateLocal(source).c_str()); }));
}

void st_render()
{
	const char *id_c;
	MLGetString(stdlink, &id_c);

	string id(id_c);
	MLReleaseString(stdlink, id_c);

	st_wrapper_exec(function<void(void)>([&]() {
		boost::optional<int> frameCount;

		int i;
		if (!MLGetInteger32(stdlink, &i))
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
			}
			else
			{
				MLClearError(stdlink);
				throw runtime_error("Could not get value for Frame");
			}
		}
		else
		{
			frameCount = boost::optional<int>(i);
		}

		// Read size params
		int width, height;
		MLGetInteger32(stdlink, &width);
		MLGetInteger32(stdlink, &height);

		// Read mouse param
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

		// Read format
		const char *formatName;
		GLenum format;
		if (!MLGetString(stdlink, &formatName))
			throw runtime_error("Invalid Format parameter");

		if (strcmp(formatName, "RGBA") == 0)
			format = GL_RGBA;
		else if (strcmp(formatName, "RGB") == 0)
			format = GL_RGB;
		else if (strcmp(formatName, "Luminance") == 0)
			format = GL_LUMINANCE;
		else
		{
			MLReleaseString(stdlink, formatName);
			throw runtime_error("Invalid Format parameter");
		}

		MLReleaseString(stdlink, formatName);

		// Render image
		auto image(host.Render(id, frameCount, width, height, mouse, format));

		MLPutFunction(stdlink, "Image", 1);
		MLPutReal32Array(stdlink, image->data->data(), &image->dims[0], NULL, 3);
	}));
}

// Shadertoy module entrypoint
int st_main(int argc, char *argv[])
{
	host.Allocate();
	return MLMain(argc, argv);
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
