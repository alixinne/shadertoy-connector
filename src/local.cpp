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

shadertoy::BufferConfig getBufferConfig(const string &shaderId, const pair<string, string> &bufferSource)
{
	// tmp directory
	fs::path basedir(fs::temp_directory_path());

	// Resulting config
	shadertoy::BufferConfig buffer;

	// Get lowercase name as buffer name
	buffer.name = bufferSource.first;
	transform(buffer.name.begin(), buffer.name.end(), buffer.name.begin(), ::tolower);

	// Write shader code in file
	stringstream sspath;
	sspath << "stcode_" << shaderId << "-" << buffer.name << ".glsl";
	fs::path p(basedir / sspath.str());

	ofstream ofs(p.string());
	ofs << bufferSource.second;
	ofs.close();

	// Register file in config
	buffer.shaderFiles.push_back(p);

	return buffer;
}

void loadLocal(const string &shaderId, const map<string, string> &bufferSources, shadertoy::ContextConfig &contextConfig)
{
	// Put everything in tmp
	fs::path basedir(fs::temp_directory_path());

	// The image buffer should be last
	auto imageIt = bufferSources.find("image");
	if (imageIt == bufferSources.end())
		throw runtime_error("Could not find the image buffer for the local context.");

	try
	{
		// Add all auxiliary buffers
		for (auto it = bufferSources.begin(); it != bufferSources.end(); ++it)
		{
			if (it != imageIt)
			{
				shadertoy::BufferConfig buffer(getBufferConfig(shaderId, *it));
				contextConfig.bufferConfigs.insert(make_pair(buffer.name, buffer));
			}
		}

		// Add image buffer
		shadertoy::BufferConfig imageBuffer(getBufferConfig(shaderId, *imageIt));
		contextConfig.bufferConfigs.insert(make_pair(imageBuffer.name, imageBuffer));
	}
	catch (exception &ex)
	{
		// Rethrow
		throw runtime_error(ex.what());
	}
}
