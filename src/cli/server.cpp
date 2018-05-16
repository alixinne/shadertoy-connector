#include "server.hpp"

#include <shadertoy/spdlog/spdlog.h>

int main(int argc, char *argv[])
{
	spdlog::set_level(spdlog::level::debug);

	server srv("tcp://*:13710");
	srv.run();
	return 0;
}

