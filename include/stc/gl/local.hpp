#ifndef _STC_GL_LOCAL_HPP_
#define _STC_GL_LOCAL_HPP_

#include <string>

#include <shadertoy.hpp>

namespace stc
{
namespace gl
{

void load_local(const std::string &shaderId, const std::vector<std::pair<std::string, std::string>> &bufferSources,
				shadertoy::render_context &context, shadertoy::swap_chain &chain,
				const shadertoy::rsize &render_size);
}
}

#endif /* _STC_GL_LOCAL_HPP_ */
