#include <iostream>
#include <sstream>
#include <string>

#include <epoxy/egl.h>
#include <epoxy/gl.h>

#include "context.hpp"
#include "host.hpp"

using namespace std;

void GetFirstNMessages(GLuint numMsgs)
{
	GLint maxMsgLen = 0;
	glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);

	std::vector<GLchar> msgData(numMsgs * maxMsgLen);
	std::vector<GLenum> sources(numMsgs);
	std::vector<GLenum> types(numMsgs);
	std::vector<GLenum> severities(numMsgs);
	std::vector<GLuint> ids(numMsgs);
	std::vector<GLsizei> lengths(numMsgs);

	GLuint numFound = glGetDebugMessageLog(numMsgs, msgData.size(), &sources[0], &types[0], &ids[0],
										   &severities[0], &lengths[0], &msgData[0]);

	sources.resize(numFound);
	types.resize(numFound);
	severities.resize(numFound);
	ids.resize(numFound);
	lengths.resize(numFound);

	std::vector<std::string> messages;
	messages.reserve(numFound);

	std::vector<GLchar>::iterator currPos = msgData.begin();
	for (size_t msg = 0; msg < lengths.size(); ++msg)
	{
		messages.push_back(std::string(currPos, currPos + lengths[msg] - 1));
		currPos = currPos + lengths[msg];
	}

	for (auto &msg : messages)
	{
		cerr << "GL: " << msg << endl;
	}
}

int main(int argc, char *argv[])
{
	cerr << "Shadertoy connector testing program." << endl;

	try
	{
		Host host;

		cerr << "Trying to allocate the host context." << endl;
		host.Allocate();

		cerr << "Allocation succeeded, we are using OpenGL on " << glGetString(GL_VENDOR) << " "
			 << glGetString(GL_RENDERER) << " " << glGetString(GL_VERSION) << " "
			 << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

		cerr << "The following extensions are available: " << glGetString(GL_EXTENSIONS) << endl;


		cerr << "Now attempting to create a local context." << endl;

		string source = "void mainImage(out vec4 O, in vec2 U) \
		{ O = vec4(length(U)/iResolution.x); }";

		cerr << "Source: " << endl << source << endl << endl;

		string ctx = host.CreateLocal(source);
		cerr << "Created the context with id " << ctx << endl;

		cerr << "Now attempting to render the context." << endl;

		StImage *img = nullptr;
		float mouse[4] = { 0.f };
		img = host.Render(ctx, 0, 100, 100, mouse, GL_RGBA);

		cerr << "The test completed successfully." << endl;
	}
	catch (std::runtime_error &ex)
	{
		cerr << "An error occurred: " << ex.what() << endl;
		GetFirstNMessages(10);
		exit(1);
	}

	return 0;
}

#ifdef _MSC_VER
#include <Windows.h>

int WinMain(HINSTANCE hinstCurrent, HINSTANCE hinstPrevious, LPSTR lpszCmdLine, int nCmdShow)
{
	return main(0, NULL);
}

#endif
