#ifndef _BASIC_CONTEXT_HPP_
#define _BASIC_CONTEXT_HPP_

#include <string>

#include <epoxy/gl.h>

#include <boost/variant.hpp>

#include "image.hpp"

class basic_context
{
	/// Context identifier
	std::string id_;

protected:
	/**
	 * @brief Initializes a new basic_context with the given \p id
	 *
	 * @param id Unique identifier for the context
	 */
	basic_context(const std::string &id);

public:
	virtual ~basic_context();

	/**
	 * @brief Gets the identifier for this context
	 *
	 * @return String identifying this context
	 */
	inline const std::string id() const
	{ return id_; }

	/**
	 * @brief Gets the number of rendered frames using this context
	 *
	 * @return Number of rendered frames
	 */
	virtual int frame_count() const = 0;

	/**
	 * @brief Renders a new frame at the given resolution
	 *
	 * @param frame  Number of the frame to render
	 * @param width  Rendering width
	 * @param height Rendering height
	 * @param mouse  Mouse status
	 * @param format Rendering format
	 */
	virtual void perform_render(int frame, size_t width, size_t height, const float mouse[4], GLenum format) = 0;

	/**
	 * @brief Gets the current frame result
	 *
	 * @return Handle to the frame result
	 */
	virtual StImage current_image() const = 0;

	/**
	 * Sets the value of an input for the next renderings.
	 *
	 * @param buffer   Name of the buffer to change the inputs
	 * @param channel  Channel id (0 to 3) of the input to change
	 * @param data     Image data (or buffer name) to feed to the channel
	 */
	virtual void set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<StImage>> &data) = 0;

	/**
	 * Sets the filter for a given input.
	 *
	 * @param buffer    Name of the buffer to change the inputs
	 * @param channel   Channel id (0 to 3) of the input to change
	 * @param minFilter Filtering method for the given input
	 */
	virtual void set_input_filter(const std::string &buffer, size_t channel, GLint minFilter) = 0;

	/**
	 * Resets the values of the given input to the default as given by the context.
	 *
	 * @param buffer  Name of the buffer to change the inputs
	 * @param channel Channel id (0 to 3) of the input to reset
	 */
	virtual void reset_input(const std::string &buffer, size_t channel) = 0;
};

#endif /* _BASIC_CONTEXT_HPP_ */
