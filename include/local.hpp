#ifndef _LOCAL_HPP_
#define _LOCAL_HPP_

#include <string>

#include <shadertoy.hpp>

void loadLocal(const std::string &shaderId, const std::vector<std::pair<std::string, std::string>> &bufferSources,
			   shadertoy::render_context &context, shadertoy::swap_chain &chain,
			   const shadertoy::rsize &render_size);

#endif /* _LOCAL_HPP_ */
