#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <curl/curl.h>
#include <json/json.h>

#include <boost/filesystem.hpp>

#include <epoxy/gl.h>

#include "getpid.h"

#include "remote.hpp"

using namespace std;
namespace fs = boost::filesystem;

void initRemote()
{
	// Init CURL
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

void freeRemote()
{
	// Cleanup CURL
	curl_global_cleanup();
}

size_t curl_write_data(char *buffer, size_t size, size_t nmemb, void *userp)
{
	ostream &ss = *static_cast<ostream *>(userp);
	size_t sz = size * nmemb;
	ss.write(buffer, sz);
	return sz;
}

void file_get(CURL *curl, const string &url, const fs::path &dst)
{
	ofstream ofs(dst.string(), ios::out | ios::binary);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		throw runtime_error(curl_easy_strerror(res));
	}

	ofs.close();
}

stringstream curl_get(CURL *curl, const string &url)
{
	stringstream ss;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		throw runtime_error(curl_easy_strerror(res));
	}

	return ss;
}

Json::Value json_get(CURL *curl, const string &url)
{
	Json::Value result;
	curl_get(curl, url) >> result;
	return result;
}

void loadRemote(const string &shaderId, const string &shaderApiKey, shadertoy::ContextConfig &contextConfig)
{
	CURL *curl = curl_easy_init();

	// Put everything in tmp
	fs::path basedir(fs::temp_directory_path());

	if (!curl)
	{
		throw runtime_error("Failed to initialize curl");
	}

	try
	{
		string endpoint =
		string("https://www.shadertoy.com/api/v1/shaders/") + shaderId + string("?key=") + shaderApiKey;
		Json::Value shaderSpec = json_get(curl, endpoint);

		// Check errors from ShaderToy
		if (!shaderSpec["Error"].isNull())
		{
			throw runtime_error(shaderSpec["Error"].asString().c_str());
		}

		ofstream dump((basedir / fs::path(shaderId + string(".json"))).string());
		dump << shaderSpec;
		dump.close();

		// Create buffer configs for each render pass
		regex rxChar("\\bchar\\b");

		for (size_t i = 0; i < shaderSpec["Shader"]["renderpass"].size(); ++i)
		{
			auto &pass(shaderSpec["Shader"]["renderpass"][static_cast<int>(i)]);

			// Create buffer
			shadertoy::BufferConfig imageBuffer;

			// Get buffer name from JSON object
			imageBuffer.name = pass["name"].asString();
			if (imageBuffer.name.empty())
				imageBuffer.name = pass["type"].asString();

			// Lowercase buffer name
			std::transform(imageBuffer.name.begin(), imageBuffer.name.end(),
				imageBuffer.name.begin(), ::tolower);

			// Skip if sound buffer
			if (pass["type"].asString().compare("sound") == 0)
			{
				continue;
			}

			// Load code
			stringstream sspath;
			sspath << "stcode_remoteshader-" << getpid() << "-" << shaderId << "-" << i << ".glsl";
			fs::path p(basedir / sspath.str());

			ofstream ofs(p.string());
			ofs << regex_replace(pass["code"].asString(), rxChar, "webgl_char");
			ofs.close();

			// Load inputs
			for (size_t j = 0; j < pass["inputs"].size(); ++j)
			{
				auto &input(pass["inputs"][static_cast<int>(j)]);
				auto &conf(imageBuffer.inputConfig[input["channel"].asInt()]);
				auto &sampler(input["sampler"]);

				stringstream ssname;
				ssname << imageBuffer.name << "." << input["channel"].asInt();
				conf.id = ssname.str();

				if (sampler["filter"].compare("mipmap") == 0)
				{
					conf.minFilter = GL_LINEAR_MIPMAP_LINEAR;
					conf.magFilter = GL_LINEAR;
				}
				else if (sampler["filter"].compare("linear") == 0)
				{
					conf.minFilter = GL_LINEAR;
					conf.magFilter = GL_LINEAR;
				}
				else if (sampler["filter"].compare("nearest") == 0)
				{
					conf.minFilter = GL_NEAREST;
					conf.magFilter = GL_NEAREST;
				}

				if (sampler["wrap"].compare("repeat") == 0)
				{
					conf.wrap = GL_REPEAT;
				}
				else if (sampler["wrap"].compare("clamp") == 0)
				{
					conf.wrap = GL_CLAMP_TO_EDGE;
				}

				conf.vflip = sampler["vflip"].compare("true") == 0;

				if (input["ctype"].compare("texture") == 0 || input["ctype"].compare("cubemap") == 0)
				{
					conf.type = "texture";

					fs::path srcpath(input["src"].asString());
					string url = string("https://www.shadertoy.com") + input["src"].asString();
					fs::path dstpath(basedir / fs::path(std::string("stinput_") + srcpath.filename().string()));

					if (!fs::exists(dstpath))
					{
						file_get(curl, url, dstpath);
					}

					conf.source = dstpath.string();
				}
				else if (input["ctype"].compare("buffer") == 0)
				{
					conf.type = "buffer";
					conf.source = "Buf A";
					conf.source.back() = 'A' + (input["id"].asInt() - 257);
				}
				else
				{
					stringstream ss;
					ss << "Unsupported input " << input["ctype"].asString() << " for pass " << i
					   << ", input " << input["channel"].asInt();

					if (!(input["ctype"].compare("keyboard") == 0))
					{
						// throw runtime_error(ss.str().c_str());
					}
				}
			}

			// Add to context
			imageBuffer.shaderFiles.push_back(p);
			contextConfig.bufferConfigs.insert(make_pair(imageBuffer.name, imageBuffer));
		}

		// Image buffer should be last
		pair<string, shadertoy::BufferConfig> imagebuf(*contextConfig.bufferConfigs.begin());
		contextConfig.bufferConfigs.erase(contextConfig.bufferConfigs.begin());
		contextConfig.bufferConfigs.insert(imagebuf);
	}
	catch (exception &ex)
	{
		// Free CURL
		curl_easy_cleanup(curl);

		// Rethrow
		throw runtime_error(ex.what());
	}

	// Free CURL
	curl_easy_cleanup(curl);
}
