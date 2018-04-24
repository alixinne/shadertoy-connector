#ifndef _REMOTE_HPP_
#define _REMOTE_HPP_

#include <string>

#include <shadertoy.hpp>

void initRemote();
void freeRemote();

void loadRemote(const std::string &shaderId, const std::string &shaderApiKey, shadertoy::render_context &context,
				shadertoy::swap_chain &chain, const shadertoy::rsize &render_size);

#endif /* _REMOTE_HPP_ */
