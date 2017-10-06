#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <curl/curl.h>
#include <json/json.h>

#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include <GL/glew.h>

#include <oglplus/enumerations.hpp>

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
	ofstream ofs(dst.string());

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		throw runtime_error(curl_easy_strerror(res));
	}
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

int loadRemote(const string &shaderId, const string &shaderApiKey, shadertoy::ContextConfig &contextConfig)
{
	CURL *curl = curl_easy_init();

	if (!curl)
	{
		BOOST_LOG_TRIVIAL(error) << "Failed to initialize curl.";
		curl_global_cleanup();
		return 100;
	}

	int code = 0;
	try
	{
		string endpoint =
		string("https://www.shadertoy.com/api/v1/shaders/") + shaderId + string("?key=") + shaderApiKey;
		BOOST_LOG_TRIVIAL(info) << "Fetching shader info from " << endpoint;
		Json::Value shaderSpec = json_get(curl, endpoint);

		// Check errors from ShaderToy
		if (!shaderSpec["Error"].isNull())
		{
			throw runtime_error(shaderSpec["Error"].asString().c_str());
		}

		ofstream dump(shaderId + string(".json"));
		dump << shaderSpec;
		dump.close();

		// Create buffer configs for each render pass
		regex rxiGlobalTime("\\biGlobalTime\\b");

		for (int i = 0; i < shaderSpec["Shader"]["renderpass"].size(); ++i)
		{
			auto &pass(shaderSpec["Shader"]["renderpass"][i]);

			// Create buffer
			shadertoy::BufferConfig imageBuffer;
			imageBuffer.name = pass["name"].asString();
			if (imageBuffer.name.empty())
				imageBuffer.name = pass["type"].asString();

			// Skip if sound buffer
			if (pass["type"].asString().compare("sound") == 0)
			{
				BOOST_LOG_TRIVIAL(warning) << "Skipping unsupported sound shader.";
				continue;
			}

			// Load code
			stringstream sspath;
			sspath << shaderId << "-" << i << ".glsl";
			fs::path p(sspath.str());

			if (!fs::exists(p))
			{
				ofstream ofs(p.string());
				ofs << regex_replace(pass["code"].asString(), rxiGlobalTime, "iTime");
				ofs.close();
			}

			// Load inputs
			for (int j = 0; j < pass["inputs"].size(); ++j)
			{
				auto &input(pass["inputs"][j]);
				auto &conf(imageBuffer.inputConfig[input["channel"].asInt()]);
				auto &sampler(input["sampler"]);

				stringstream ssname;
				ssname << imageBuffer.name << "." << input["channel"].asInt();
				conf.id = ssname.str();

				if (sampler["filter"].compare("mipmap") == 0)
				{
					conf.minFilter = oglplus::TextureMinFilter::LinearMipmapLinear;
					conf.magFilter = oglplus::TextureMagFilter::Linear;
				}
				else if (sampler["filter"].compare("linear") == 0)
				{
					conf.minFilter = oglplus::TextureMinFilter::Linear;
					conf.magFilter = oglplus::TextureMagFilter::Linear;
				}
				else if (sampler["filter"].compare("nearest") == 0)
				{
					conf.minFilter = oglplus::TextureMinFilter::Nearest;
					conf.magFilter = oglplus::TextureMagFilter::Nearest;
				}

				if (sampler["wrap"].compare("repeat") == 0)
				{
					conf.wrap = oglplus::TextureWrap::Repeat;
				}
				else if (sampler["wrap"].compare("clamp") == 0)
				{
					conf.wrap = oglplus::TextureWrap::ClampToEdge;
				}

				conf.vflip = sampler["vflip"].compare("true") == 0;

				if (input["ctype"].compare("texture") == 0 || input["ctype"].compare("cubemap") == 0)
				{
					conf.type = "texture";

					fs::path srcpath(input["src"].asString());
					string url = string("https://www.shadertoy.com") + input["src"].asString();
					fs::path dstpath(srcpath.filename());

					if (!fs::exists(dstpath))
					{
						BOOST_LOG_TRIVIAL(info) << "Downloading " << url;
						file_get(curl, url, dstpath);
					}
					else
					{
						BOOST_LOG_TRIVIAL(info) << "Using cache for " << url;
					}

					conf.source = dstpath.string();
				}
				else if (input["ctype"].compare("buffer") == 0)
				{
					conf.type = "buffer";
					conf.source = "Buf A";
					conf.source.back() = 'A' + (input["id"].asInt() - 257);

					BOOST_LOG_TRIVIAL(debug)
					<< "Pass " << i << ", input " << input["channel"].asInt() << ": binding "
					<< conf.source << " buffer " << conf.enabled();
				}
				else
				{
					stringstream ss;
					ss << "Unsupported input " << input["ctype"].asString() << " for pass " << i
					   << ", input " << input["channel"].asInt();

					if (!(input["ctype"].compare("keyboard") == 0))
					{
						throw runtime_error(ss.str().c_str());
					}
					else
					{
						BOOST_LOG_TRIVIAL(warning) << ss.str();
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
		BOOST_LOG_TRIVIAL(error) << ex.what();
		code = 101;
	}

	// Free CURL
	curl_easy_cleanup(curl);

	return code;
}
