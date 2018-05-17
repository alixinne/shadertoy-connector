#ifndef _STC_CORE_BASIC_CONTEXT_HPP_
#define _STC_CORE_BASIC_CONTEXT_HPP_

#include <string>

#include <epoxy/gl.h>

#include <boost/variant.hpp>

#include "stc/core/image.hpp"

namespace stc
{
namespace core
{

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
	 * Sets the value of an input for the next renderings.
	 *
	 * @param buffer   Name of the buffer to change the inputs
	 * @param channel  Channel id (0 to 3) of the input to change
	 * @param data     Image data (or buffer name) to feed to the channel
	 */
	virtual void set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<image>> &data) = 0;

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
}
}

#endif /* _STC_CORE_BASIC_CONTEXT_HPP_ */
