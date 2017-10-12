#ifndef _HOST_HPP_
#define _HOST_HPP_

#include <map>
#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/optional.hpp>

struct StContext;
struct StImage;

class Host
{
	public:
	/**
	 * Initialize the host object
	 */
	Host();

	/**
	 * Frees up resources used by the host object
	 */
	~Host();

	/**
	 * Allocate the rendering context. Throws exceptions
	 * on errors.
	 */
	void Allocate();

	/**
	 * Render a shadertoy by its name.
	 *
	 * @param  id     Name of the shadertoy context to render.
	 * @param  frame  Number of the frame, or no value to render the next frame.
	 * @param  width  Rendering width.
	 * @param  height Rendering height.
	 * @param  mouse  Value of the iMouse uniform.
	 * @param  format Format of the rendering (GL_RGBA, GL_RGB, or GL_LUMINANCE).
	 * @return        Pointer to the rendered frame.
	 */
	StImage *Render(const std::string &id, boost::optional<int> frame, int width, int height, float mouse[4], GLenum format);

	/**
	 * Create a new local context from the give source code.
	 *
	 * @param  source Source of the main program for this context.
	 * @return        Unique identifier for this context.
	 */
	std::string CreateLocal(const std::string &source);

	private:
	/**
	 * Get or allocate a new remote context. Throws if no local context exists
	 * for this name, and no remote context could be created for this id.
	 *
	 * @param  id Shadertoy identifier
	 * @return    Pointer to the context.
	 */
	std::shared_ptr<StContext> GetContext(const std::string &id);

	/// OpenGL window for rendering
	GLFWwindow *st_window;
	/// Number of allocated local contexts
	int local_counter;
	/// List of rendering contexts by name
	std::map<std::string, std::shared_ptr<StContext>> st_contexts;

	// Allocation state
	bool m_remoteInit;
	bool m_glfwInit;
};

#endif /* _HOST_HPP_ */
