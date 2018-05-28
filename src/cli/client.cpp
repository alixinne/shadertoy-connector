#include <iostream>

#include "stc/client/net_host.hpp"

#include "spdlog/spdlog.h"

int main(int argc, char *argv[])
{
	spdlog::set_level(spdlog::level::debug);

	stc::client::net_host client("tcp://localhost:13710");
	client.allocate();

	std::vector<std::pair<std::string, std::string>> buffer_sources;
	buffer_sources.emplace_back("image", "void mainImage(out vec4 O, in vec2 U){O=vec4(length(U/iResolution.xy));}");

	std::cerr << "Creating local context" << std::endl;
	auto context_id = client.create_local(buffer_sources);

	std::cerr << "Resetting context inputs" << std::endl;
	auto context = client.get_context(context_id);
	
	std::cerr << "Obtained context" << std::endl;
	context->reset_input("image", 0);

	std::cerr << "Rendering image" << std::endl;
	std::array<float, 4> mouse{0.f, 0.f, 0.f, 0.f};
	auto image = client.render(context_id, 0, 16, 16, mouse, GL_RGBA);

	std::cout << "First pixel value: " << std::endl
		<< "R: " << (*image.data)[0] << std::endl
		<< "G: " << (*image.data)[1] << std::endl
		<< "B: " << (*image.data)[2] << std::endl
		<< "A: " << (*image.data)[3] << std::endl;

	return 0;
}
