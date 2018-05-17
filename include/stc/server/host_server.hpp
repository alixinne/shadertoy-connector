#ifndef _STC_SERVER_HOST_SERVER_HPP_
#define _STC_SERVER_HOST_SERVER_HPP_

#include <memory>
#include <string>

namespace stc
{
namespace server
{

class host_server_impl;

class host_server
{
	host_server_impl * const impl_;

public:
	host_server(const std::string &bind_address);
	~host_server();

	void run();
};
}
}

#endif /* _STC_SERVER_HOST_SERVER_HPP_ */
