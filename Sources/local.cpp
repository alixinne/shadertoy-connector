#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include <epoxy/gl.h>

#include "local.hpp"

using namespace std;
namespace fs = boost::filesystem;

void loadLocal(const string &shaderId, const string &source, shadertoy::ContextConfig &contextConfig)
{
	// Put everything in tmp
	fs::path basedir(fs::temp_directory_path());

	try
	{
		// Create buffer
		shadertoy::BufferConfig imageBuffer;
		imageBuffer.name = "image";

		// Load code
		stringstream sspath;
		sspath << shaderId << ".glsl";
		fs::path p(basedir / sspath.str());

		ofstream ofs(p.string());
		ofs << source;
		ofs.close();

		// Add to context
		imageBuffer.shaderFiles.push_back(p);
		contextConfig.bufferConfigs.insert(make_pair(imageBuffer.name, imageBuffer));
	}
	catch (exception &ex)
	{
		// Rethrow
		throw runtime_error(ex.what());
	}
}
