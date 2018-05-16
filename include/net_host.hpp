#ifndef _NET_HOST_HPP_
#define _NET_HOST_HPP_

#include "basic_context.hpp"

#include "basic_host.hpp"

class net_host_impl;

class net_context : public basic_context
{
	net_host_impl * const impl_;

public:
	net_context(const std::string &id, net_host_impl *impl);

	void set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<StImage>> &data) override;

	void set_input_filter(const std::string &buffer, size_t channel, GLint minFilter) override;

	void reset_input(const std::string &buffer, size_t channel) override;
};

class net_host : public basic_host
{
	net_host_impl * const impl_;

public:
	net_host(const std::string &target);

	~net_host();

	void allocate() override;

	StImage render(const std::string &id, boost::optional<int> frame, size_t width, size_t height,
				   const std::array<float, 4> &mouse, GLenum format) override;

	void reset(const std::string &id) override;

	std::string create_local(const std::vector<std::pair<std::string, std::string>> &bufferSources) override;

	std::shared_ptr<basic_context> get_context(const std::string &id) override;
};

#endif /* _NET_HOST_HPP_ */
