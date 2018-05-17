#ifndef _STC_GL_CONTEXT_HPP_
#define _STC_GL_CONTEXT_HPP_

#include <memory>
#include <string>

#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy.hpp>

#include "stc/core/basic_context.hpp"

namespace stc
{
namespace gl
{

class context : public core::basic_context
{
	/// Rendering size
	shadertoy::rsize render_size_;

	/// Rendering context
	shadertoy::render_context context_;

	/// Associated swap chain
	shadertoy::swap_chain chain_;

	/// Number of rendered frames
	int frame_count_;

	/// The currently rendered image
	core::image current_image_;

public:
	/**
	 * Builds a new rendering context for a given Shadertoy.
	 *
	 * @param shaderId Identifier of the Shadertoy to render.
	 * @param width    Initial width of the rendering context.
	 * @param height   Initial height of the rendering context.
	 */
	context(const std::string &shaderId, size_t width, size_t height);

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
	context(const std::string &shaderId, const std::vector<std::pair<std::string, std::string>> &bufferSources,
			size_t width, size_t height);

	/**
	 * @brief Gets the number of rendered frames using this context
	 *
	 * @return Number of rendered frames
	 */
	inline int frame_count() const
	{ return frame_count_; }

	/**
	 * @brief Renders a new frame at the given resolution
	 *
	 * @param frame  Number of the frame to render
	 * @param width  Rendering width
	 * @param height Rendering height
	 * @param mouse  Mouse status
	 * @param format Rendering format
	 */
	void perform_render(int frame, size_t width, size_t height, const std::array<float, 4> &mouse, GLenum format);

	/**
	 * @brief Gets the current frame result
	 *
	 * @return Handle to the frame result
	 */
	inline core::image current_image() const
	{ return current_image_; }

	void set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<core::image>> &data) override;

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
	int format_depth(GLenum format);

	/**
	 * Returns the PixelDataFormat of a given image depth.
	 *
	 * @param  depth Depth of the image
	 */
	static GLint depth_format(int depth);

	/**
	 * Allocates the rendering context
	 */
	void create_context();

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
		std::shared_ptr<core::image> data_buffer_;
		std::shared_ptr<shadertoy::gl::texture> texture_;

		std::shared_ptr<shadertoy::inputs::buffer_input> member_input_;

		std::shared_ptr<shadertoy::inputs::basic_input> overriden_input_;

	protected:
		void load_input() override;

		void reset_input() override;

		std::shared_ptr<shadertoy::gl::texture> use_input() override;

	public:
		override_input(std::shared_ptr<shadertoy::inputs::basic_input> overriden_input);

		void set(std::shared_ptr<core::image> data_buffer);

		void set(std::shared_ptr<shadertoy::inputs::buffer_input> member_input);

		inline const std::shared_ptr<shadertoy::inputs::basic_input> &overriden_input() const
		{ return overriden_input_; }
	};
};
}
}

#endif /* _STC_GL_CONTEXT_HPP_ */
