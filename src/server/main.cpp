#include <iostream>

#include "stc/server/host_server.hpp"

#include "spdlog/spdlog.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	bool debug_mode;
	std::string bind_addr;

	try
	{
		po::options_description desc("shadertoy-connector server");
		desc.add_options()
			("help,h", "Show this help message")
			("debug,d", po::bool_switch(&debug_mode)->default_value(false), "Enable debug output")
			("bind,b", po::value<std::string>(&bind_addr)->default_value("tcp://*:13710"), "Endpoint to bind to");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help"))
			std::cout << desc << std::endl;
		else
		{
			if (debug_mode)
				spdlog::set_level(spdlog::level::debug);

			stc::server::host_server srv(bind_addr);
			srv.run();
		}
	}
	catch (const po::error &ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

