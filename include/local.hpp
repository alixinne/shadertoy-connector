#ifndef _LOCAL_HPP_
#define _LOCAL_HPP_

#include <string>

#include <shadertoy/Shadertoy.hpp>

void loadLocal(const std::string &shaderId, const std::map<std::string, std::string> &bufferSources,
			   shadertoy::ContextConfig &contextConfig);

#endif /* _LOCAL_HPP_ */
