#include "stc/core/image.hpp"

using namespace stc::core;

image::image()
	: data(), dims{0, 0, 0}, changed(false), frame_timing(0)
{
}

void image::alloc()
{
	size_t b = 1;
	for (auto dim : dims)
		b *= dim;

	if (!data || data->size() != b)
	{
		data = std::make_shared<std::vector<float>>(b);
	}
}
