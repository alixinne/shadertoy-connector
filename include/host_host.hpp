#ifndef _HOST_HOST_HPP_
#define _HOST_HOST_HPP_

#include "basic_host.hpp"

#include <map>
#include <memory>

class host_host
{
	/// List of host instance objects
	std::map<std::string, std::unique_ptr<basic_host>> known_hosts_;

	/// Name of the currently selected host for rendering
	std::string current_;

public:
	host_host();

	basic_host &current();

	void set_current(const std::string &identifier);
};

#endif /* _HOST_HOST_HPP_ */
