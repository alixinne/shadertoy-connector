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

	// Flag to indicate the data in the data field has changed since the last
	// rendering
	bool changed = false;
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

	/// The currently rendered image
	StImage currentImage;

	/**
	 * Builds a new rendering context for a given Shadertoy.
	 *
	 * @param shaderId Identifier of the Shadertoy to render.
	 * @param width    Initial width of the rendering context.
	 * @param height   Initial height of the rendering context.
	 */
	StContext(const std::string &shaderId, int width, int height);

	/**
	 * Builds a single-buffer rendering context from source.
	 *
	 * @param shaderId Identifier for this rendering context.
	 * @param source   Source contents for the file to render.
	 * @param width    Initial width of the rendering context.
	 * @param height   Initial height of the rendering context.
	 */
	StContext(const std::string &shaderId, const std::string &source, int width, int height);

	/**
	 * Renders the next frame of this Shadertoy into the currentImage field.
	 *
	 * @param window     OpenGL rendering window
	 * @param frameCount Id of the frame to render
	 * @param width      Width of the rendering
	 * @param height     Height of the rendering
	 * @param mouse      Values for the mouse uniform
	 * @param format     Format of the rendering
	 */
	void performRender(GLFWwindow *window, int frameCount, int width, int height, float mouse[4], GLenum format);

	/**
	 * Sets the value of an input for the next renderings.
	 *
	 * @param buffer   Name of the buffer to change the inputs
	 * @param channel  Channel id (0 to 3) of the input to change
	 * @param image    Image data to feed to the channel
	 */
	void setInput(const std::string &buffer, int channel, StImage &image);

	/**
	 * Resets the values of the given input to the default as given by the context.
	 *
	 * @param buffer  Name of the buffer to change the inputs
	 * @param channel Channel id (0 to 3) of the input to reset
	 */
	void resetInput(const std::string &buffer, int channel);

	private:
	/**
	 * Initialize the rendering context members.
	 */
	void initialize(const std::string &shaderId, int width, int height);

	/**
	 * Returns the number of components for a given format.
	 *
	 * @param  format Format to return the number of components of
	 */
	int formatDepth(GLenum format);

	/**
	 * Returns the PixelDataFormat of a given image depth.
	 *
	 * @param  depth Depth of the image
	 */
	oglplus::PixelDataFormat depthFormat(int depth);

	/**
	 * Allocates the rendering context
	 *
	 * @param config Rendering context configuration
	 */
	void createContext(shadertoy::ContextConfig &config);

	/**
	 * Gets the map of input overrides for a given buffer.
	 *
	 * @param buffer Name of the buffer to get the overrides for.
	 */
	std::map<int, StImage> &getBufferInputOverrides(const std::string &buffer);

	/// Input override map
	std::map<std::string, std::map<int, StImage>> inputOverrides;

	/// Data texture handler
	std::shared_ptr<oglplus::Texture> DataTextureHandler(const shadertoy::InputConfig &inputConfig,
		bool &skipTextureOptions,
		bool &skipCache,
		bool &framebufferSized);

	/**
	 * Get an oglplus::Texture instance for the given input id.
	 */
	std::shared_ptr<oglplus::Texture> getDataTexture(const std::string &inputId);

	/// List of input override textures OpenGL objects
	std::map<std::string, std::shared_ptr<oglplus::Texture>> textures;

	/// true if the set of input overrides has changed and textures need to be reloaded
	bool reloadInputConfig;
};

#endif /* _CONTEXT_HPP_ */
