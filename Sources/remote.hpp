#ifndef _REMOTE_HPP_
#define _REMOTE_HPP_

#include <string>

#include <shadertoy/BufferConfig.hpp>
#include <shadertoy/ContextConfig.hpp>

void initRemote();
void freeRemote();

void loadRemote(const std::string &shaderId, const std::string &shaderApiKey, shadertoy::ContextConfig &contextConfig);

#endif /* _REMOTE_HPP_ */
