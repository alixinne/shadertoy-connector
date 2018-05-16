#include "image.hpp"

StImage::StImage()
	: data(), dims{0, 0, 0}, changed(false), frameTiming(0)
{
}

void StImage::alloc()
{
	size_t b = 1;
	for (auto dim : dims)
		b *= dim;

	if (!data || data->size() != b)
	{
		data = std::make_shared<std::vector<float>>(b);
	}
}
