#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include <memory>
#include <string>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <oglplus/all.hpp>

#include <shadertoy/Shadertoy.hpp>

struct StImage
{
	std::shared_ptr<std::vector<float>> data;
	int dims[3];
};

struct StContext
{
	/// Identifier of the Shadertoy object
	std::string shaderId;

	/// Rendering context configuration
	shadertoy::ContextConfig config;

	/// Rendering context
	std::shared_ptr<shadertoy::RenderContext> context;

	/// GL wrapper
	oglplus::Context gl;

	/// Number of rendered frames
	int frameCount;

	/**
	 * Builds a new rendering context for a given Shadertoy.
	 *
	 * @param shaderId Identifier of the Shadertoy to render.
	 * @param width    Initial width of the rendering context.
	 * @param height   Initial height of the rendering context.
	 */
	StContext(const std::string &shaderId, int width, int height);

	/**
	 * Renders the next frame of this Shadertoy and sends it to the stdlink
	 * connection as a result.
	 *
	 * @param window     OpenGL rendering window
	 * @param frameCount Id of the frame to render
	 * @param width      Width of the rendering
	 * @param height     Height of the rendering
	 *
	 * @return
	 */
	StImage performRender(GLFWwindow *window, int frameCount, int width, int height);
};

#endif /* _CONTEXT_HPP_ */
