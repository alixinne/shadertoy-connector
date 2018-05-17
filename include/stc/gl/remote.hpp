#ifndef _STC_GL_REMOTE_HPP_
#define _STC_GL_REMOTE_HPP_

#include <string>

#include <shadertoy.hpp>

namespace stc
{
namespace gl
{

void init_remote();
void free_remote();

void load_remote(const std::string &shaderId, const std::string &shaderApiKey, shadertoy::render_context &context,
				 shadertoy::swap_chain &chain, const shadertoy::rsize &render_size);
}
}

#endif /* _STC_GL_REMOTE_HPP_ */
