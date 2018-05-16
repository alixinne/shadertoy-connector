#ifndef _CONTEXT_HPP_
#define _CONTEXT_HPP_

#include <memory>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy.hpp>

#include "basic_context.hpp"

class StContext : public basic_context
{
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

public:
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

	inline int frame_count() const override
	{ return frameCount; }

	inline StImage current_image() const override
	{ return currentImage; }

	void perform_render(int frameCount, size_t width, size_t height, const float mouse[4], GLenum format) override;

	void set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<StImage>> &data) override;

	void set_input_filter(const std::string &buffer, size_t channel, GLint minFilter) override;

	void reset_input(const std::string &buffer, size_t channel) override;

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
