#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <memory>
#include <string>

class server_impl;

class server
{
	server_impl * const impl_;

public:
	server(const std::string &bind_address);
	~server();

	void run();
};

#endif /* _SERVER_HPP_ */
