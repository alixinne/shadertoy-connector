#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy/Shadertoy.hpp>

#include "om_wrapper.h"

#include "context.hpp"
#include "host.hpp"

/// Shadertoy Connector context host
Host host;

bool impl_st_parse_input(std::string &inputSpecName, std::string &buffer, int &channel)
{
	auto dotPos(inputSpecName.find('.'));
	if (dotPos == std::string::npos)
	{
		std::stringstream ss;
		ss << "Invalid input specification " << inputSpecName;
		throw std::runtime_error(ss.str());
	}

	buffer.assign(inputSpecName.begin(), inputSpecName.begin() + dotPos);
	std::string inputChannelStr(inputSpecName.begin() + dotPos + 1, inputSpecName.end());
	channel = std::atoi(inputChannelStr.c_str());

	if (channel < 0 || channel > 3)
	{
		std::stringstream ss;
		ss << "Invalid channel number " << inputChannelStr;
		throw std::runtime_error(ss.str());
	}

	return true;
}
