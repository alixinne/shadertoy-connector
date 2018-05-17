#ifndef _STC_HOST_MANAGER_HPP_
#define _STC_HOST_MANAGER_HPP_

#include "stc/core/basic_host.hpp"

#include <map>
#include <memory>

namespace stc
{

class host_manager
{
	/// List of host instance objects
	std::map<std::string, std::unique_ptr<core::basic_host>> known_hosts_;

	/// Name of the currently selected host for rendering
	std::string current_;

public:
	host_manager();

	core::basic_host &current();

	void set_current(const std::string &identifier);
};
}

#endif /* _STC_HOST_MANAGER_HPP_ */
