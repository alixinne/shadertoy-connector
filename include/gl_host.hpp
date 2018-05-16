#ifndef _GL_HOST_HPP_
#define _GL_HOST_HPP_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>
#include <epoxy/gl.h>

#include <boost/optional.hpp>

#include "basic_host.hpp"

class StContext;

class gl_host : public basic_host
{
	public:
	/**
	 * Initialize the host object
	 */
	gl_host();

	/**
	 * Frees up resources used by the host object
	 */
	~gl_host();

	/**
	 * Allocate the rendering context. Throws exceptions
	 * on errors.
	 */
	void allocate();

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
	StImage render(const std::string &id, boost::optional<int> frame, size_t width, size_t height,
				   const float mouse[4], GLenum format) override;

	/**
	 * Resets the context associated with this Shadertoy Id.
	 *
	 * @param id Identifier of the context.
	 */
	void reset(const std::string &id) override;

	/**
	 * Create a new local context from the give source code.
	 *
	 * @param  bufferSources Sources for all the buffers of the local context to create. An "image"
	 * buffer must be present.
	 * @return        Unique identifier for this context.
	 */
	std::string create_local(const std::vector<std::pair<std::string, std::string>> &bufferSources) override;

	/**
	 * Get or allocate a new remote context. Throws if no local context exists
	 * for this name, and no remote context could be created for this id.
	 *
	 * @param  id Shadertoy identifier
	 * @return    Pointer to the context.
	 */
	std::shared_ptr<basic_context> get_context(const std::string &id) override;

	private:
	/**
	 * Instantiate a new context from the given arguments.
	 */
	template <class... T> inline std::shared_ptr<basic_context> new_context(T &&... args)
	{
		// Get default size
		int width, height;
		glfwGetFramebufferSize(st_window, &width, &height);

		// Allocate context
		auto ptr(std::make_shared<StContext>(std::forward<T>(args)..., width, height));

		// Add to context map
		st_contexts.insert(std::make_pair(ptr->id(), ptr));

		return ptr;
	}

	/// OpenGL window for rendering
	GLFWwindow *st_window;
	/// Number of allocated local contexts
	int local_counter;
	/// List of rendering contexts by name
	std::map<std::string, std::shared_ptr<basic_context>> st_contexts;

	// Allocation state
	bool m_remoteInit;
	bool m_glfwInit;
};

#endif /* _GL_HOST_HPP_ */
