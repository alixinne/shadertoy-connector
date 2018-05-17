#include <sstream>

#include "stc/host_manager.hpp"

#include "stc/gl/host.hpp"

#if SHADERTOY_CONNECTOR_HAS_ZMQ
#include "stc/client/net_host.hpp"
#endif

using namespace stc;

host_manager::host_manager()
	: known_hosts_(),
	current_("local")
{
}

core::basic_host &host_manager::current()
{
	auto it = known_hosts_.find(current_);
	if (it == known_hosts_.end())
	{
		std::unique_ptr<core::basic_host> new_host;

		if (current_.compare("local") == 0)
		{
			new_host = std::make_unique<gl::host>();
		}
		else
		{
#if SHADERTOY_CONNECTOR_HAS_ZMQ
			new_host = std::make_unique<client::net_host>(current_);
#else
			std::stringstream ss;
			ss << "Cannot create host object for " << current_ << " because libzmq support is not enabled";
			throw std::runtime_error(ss.str());
#endif /* SHADERTOY_CONNECTOR_HAS_ZMQ */
		}

		known_hosts_.emplace(current_, std::move(new_host));

		it = known_hosts_.find(current_);
		it->second->allocate();
	}

	return *it->second;
}

void host_manager::set_current(const std::string &identifier)
{
	current_ = identifier;
}

