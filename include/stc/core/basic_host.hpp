#ifndef _STC_CORE_BASIC_HOST_HPP_
#define _STC_CORE_BASIC_HOST_HPP_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

#include <epoxy/gl.h>

#include "image.hpp"

namespace stc
{
namespace core
{

class basic_context;

class basic_host
{
	public:
	/**
	 * Initialize the host object
	 */
	basic_host();

	/**
	 * Frees up resources used by the host object
	 */
	virtual ~basic_host();

	/**
	 * Allocate the rendering context. Throws exceptions on errors.
	 */
	virtual void allocate() = 0;

	/**
	 * Render a shadertoy by its name.
	 *
	 * @param  id     Name of the shadertoy context to render.
	 * @param  frame  Number of the frame, or no value to render the next frame.
	 * @param  width  Rendering width.
	 * @param  height Rendering height.
	 * @param  mouse  Value of the iMouse uniform.
	 * @param  format Format of the rendering (GL_RGBA, GL_RGB, or GL_LUMINANCE).
	 * @return        Handle to the rendered frame
	 */
	virtual image render(const std::string &id, boost::optional<int> frame, size_t width, size_t height,
						 const std::array<float, 4> &mouse, GLenum format) = 0;

	/**
	 * Resets the context associated with this Shadertoy Id.
	 *
	 * @param id Identifier of the context.
	 */
	virtual void reset(const std::string &id) = 0;

	/**
	 * Create a new local context from the give source code.
	 *
	 * @param  bufferSources Sources for all the buffers of the local context to create. An "image"
	 * buffer must be present.
	 * @return        Unique identifier for this context.
	 */
	virtual std::string create_local(const std::vector<std::pair<std::string, std::string>> &bufferSources) = 0;

	/**
	 * Get or allocate a new remote context. Throws if no local context exists
	 * for this name, and no remote context could be created for this id.
	 *
	 * @param  id Shadertoy identifier
	 * @return    Pointer to the context.
	 */
	virtual std::shared_ptr<basic_context> get_context(const std::string &id) = 0;
};
}
}

#endif /* _STC_CORE_BASIC_HOST_HPP_ */
