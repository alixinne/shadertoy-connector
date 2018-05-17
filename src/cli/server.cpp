#include <iostream>

#include "server.hpp"

#include <shadertoy/spdlog/spdlog.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	bool debug_mode;

	try
	{
		po::options_description desc("shadertoy-connector server");
		desc.add_options()
			("help,h", "Show this help message")
			("debug", po::bool_switch(&debug_mode)->default_value(false), "Enable debug output");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help"))
			std::cout << desc << std::endl;
		else
		{
			if (debug_mode)
				spdlog::set_level(spdlog::level::debug);

			server srv("tcp://*:13710");
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

