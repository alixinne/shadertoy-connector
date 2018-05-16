#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include <memory>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy.hpp>

struct StImage
{
	std::shared_ptr<std::vector<float>> data;
	std::vector<int> dims;

	// Flag to indicate the data in the data field has changed since the last
	// rendering
	bool changed;

	// Rendering duration of the main buffer
	unsigned long long frameTiming;

	StImage();
};

struct StContext
{
	/// Identifier of the Shadertoy object
	std::string shaderId;

	/// Rendering size
	shadertoy::rsize render_size;

	/// Rendering context
	shadertoy::render_context context;

	/// Associated swap chain
	shadertoy::swap_chain chain;

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
	StContext(const std::string &shaderId, size_t width, size_t height);

	/**
	 * Builds a multi-buffer rendering context from source.
	 *
	 * @param shaderId      Identifier for this rendering context.
	 * @param bufferSources Source for all of the buffers to render. The image
	 *                      buffer must be present.
	 * @param width         Initial width of the rendering context.
	 * @param height        Initial height of the rendering context.
	 * @throws std::runtime_error If an error occurs during the construction of
	 *                            the local context, or if the image buffer is
	 *                            missing from the definition.
	 */
	StContext(const std::string &shaderId, const std::vector<std::pair<std::string, std::string>> &bufferSources,
			  size_t width, size_t height);

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
	void performRender(GLFWwindow *window, int frameCount, size_t width, size_t height, const float mouse[4], GLenum format);

	/**
	 * Sets the value of an input for the next renderings.
	 *
	 * @param buffer   Name of the buffer to change the inputs
	 * @param channel  Channel id (0 to 3) of the input to change
	 * @param data     Image data (or buffer name) to feed to the channel
	 */
	void setInput(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<StImage>> &data);

	/**
	 * Sets the filter for a given input.
	 *
	 * @param buffer    Name of the buffer to change the inputs
	 * @param channel   Channel id (0 to 3) of the input to change
	 * @param minFilter Filtering method for the given input
	 */
	void setInputFilter(const std::string &buffer, size_t channel, GLint minFilter);

	/**
	 * Resets the values of the given input to the default as given by the context.
	 *
	 * @param buffer  Name of the buffer to change the inputs
	 * @param channel Channel id (0 to 3) of the input to reset
	 */
	void resetInput(const std::string &buffer, size_t channel);

	private:
	/**
	 * Initialize the rendering context members.
	 */
	void initialize(const std::string &shaderId, size_t width, size_t height);

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
	static GLint depthFormat(int depth);

	/**
	 * Allocates the rendering context
	 */
	void createContext();

	/**
	 * @brief Find a buffer by name
	 *
	 * @param name Name of the buffer to find
	 *
	 * @return Pointer to the buffer with the id \p name
	 *
	 * @throws std::runtime_error When a suitable buffer could not be found
	 */
	std::shared_ptr<shadertoy::buffers::toy_buffer> getBuffer(const std::string &name);

	class override_input : public shadertoy::inputs::basic_input
	{
		std::shared_ptr<StImage> data_buffer_;
		std::shared_ptr<shadertoy::gl::texture> texture_;

		std::shared_ptr<shadertoy::inputs::buffer_input> member_input_;

		std::shared_ptr<shadertoy::inputs::basic_input> overriden_input_;

	protected:
		void load_input() override;

		void reset_input() override;

		std::shared_ptr<shadertoy::gl::texture> use_input() override;

	public:
		override_input(std::shared_ptr<shadertoy::inputs::basic_input> overriden_input);

		void set(std::shared_ptr<StImage> data_buffer);

		void set(std::shared_ptr<shadertoy::inputs::buffer_input> member_input);

		inline const std::shared_ptr<shadertoy::inputs::basic_input> &overriden_input() const
		{ return overriden_input_; }
	};
};

#endif /* _CONTEXT_HPP_ */
