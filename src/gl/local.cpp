#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include <epoxy/gl.h>

#include "stc/gl/local.hpp"

namespace fs = boost::filesystem;

using namespace stc;
using namespace stc::gl;

std::shared_ptr<shadertoy::buffers::toy_buffer> get_buffer(const std::string &shaderId, const std::pair<std::string, std::string> &bufferSource)
{
	// tmp directory
	fs::path basedir(fs::temp_directory_path());

	// Get lowercase name as buffer name
	auto buffer_name(bufferSource.first);
	transform(buffer_name.begin(), buffer_name.end(), buffer_name.begin(), ::tolower);

	// Write shader code in file
	std::stringstream sspath;
	sspath << "stcode_" << shaderId << "-" << buffer_name << ".glsl";
	fs::path p(basedir / sspath.str());

	std::ofstream ofs(p.string());
	ofs << bufferSource.second;
	ofs.close();

	// Create buffer
	auto buffer(std::make_shared<shadertoy::buffers::toy_buffer>(buffer_name));

	// Add source files
	buffer->source_files().push_back(p.string());

	// Create shadertoy inputs
	for (size_t i = 0; i < SHADERTOY_ICHANNEL_COUNT; ++i)
		buffer->inputs().emplace_back();

	return buffer;
}

void stc::gl::load_local(const std::string &shaderId, const std::vector<std::pair<std::string, std::string>> &bufferSources,
						 shadertoy::render_context &context, shadertoy::swap_chain &chain, const shadertoy::rsize &render_size)
{
	// Put everything in tmp
	fs::path basedir(fs::temp_directory_path());

	// The default buffer should be last, and it should be named image
	// TODO: is this still relevant on 1.0.0?
	assert(bufferSources.back().first == "image");

	try
	{
		// Apply overrides to the template specification
		for (auto it = bufferSources.begin(); it != bufferSources.end(); ++it)
		{
			bool is_override = it->first.find(":") != std::string::npos;
			if (is_override)
			{
				// Make sure the part ends with a newline
				auto contents(it->second);
				if (contents.back() != '\n')
					contents += "\n";

				context.buffer_template().replace(it->first, shadertoy::compiler::template_part(it->first, contents));
			}
		}

		// Add all auxiliary buffers
		for (auto it = bufferSources.begin(); it != bufferSources.end(); ++it)
		{
			bool is_override = it->first.find(":") != std::string::npos;
			if (!is_override)
			{
				auto buffer(get_buffer(shaderId, *it));
				chain.emplace_back(buffer, shadertoy::make_size_ref(render_size));
			}
		}
	}
	catch (std::exception &ex)
	{
		// Rethrow
		throw std::runtime_error(ex.what());
	}
}
