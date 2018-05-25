#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include <array>
#include <memory>
#include <vector>

namespace stc
{
namespace core
{

struct image
{
	std::shared_ptr<std::vector<float>> data;
	std::array<uint32_t, 3> dims;

	// Flag to indicate the data in the data field has changed since the last
	// rendering
	bool changed;

	// Rendering duration of the main buffer
	uint64_t frame_timing;

	void alloc();

	image();
};
}
}

#endif /* _IMAGE_HPP_ */
