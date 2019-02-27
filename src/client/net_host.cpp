#include <iostream>

#include "stc/client/net_host.hpp"
#include "stc/net/io.hpp"

#include <zmq.hpp>

#include <shadertoy/spdlog/spdlog.h>
#include <shadertoy/spdlog/fmt/ostr.h>

#undef min

using namespace stc;
using namespace stc::client;

namespace stc
{
namespace client
{

struct net_host_impl
{
	const std::string target_address;

	zmq::context_t context;
	zmq::socket_t socket;

	std::shared_ptr<spdlog::logger> log;

	net::io io;

	net_host_impl(const std::string &target)
		: target_address(target),
		context(1),
		socket(context, ZMQ_REQ),
		log(spdlog::stderr_color_mt("shadertoy-client")),
		io(log, socket)
	{
	}

	void connect()
	{
		log->info("Connecting to {}", target_address);
		socket.connect(target_address);
	}
};
}
}

net_context::net_context(const std::string &id, net_host_impl *impl)
	: core::basic_context(id),
	impl_(impl)
{
}

void net_context::set_input(const std::string &buffer, size_t channel, const boost::variant<std::string, std::shared_ptr<core::image>> &data)
{
	impl_->io.send_string("context", ZMQ_SNDMORE);
	impl_->io.send_string(id(), ZMQ_SNDMORE);
	impl_->io.send_string("set_input", ZMQ_SNDMORE);

	// Send input specification
	impl_->io.send_string(buffer, ZMQ_SNDMORE);
	impl_->io.send_data<uint8_t>(channel, ZMQ_SNDMORE);

	// Send contents
	if (const auto img = boost::get<const std::shared_ptr<core::image>>(&data))
	{
		impl_->io.send_string("image", ZMQ_SNDMORE);
		impl_->io.send_data_noout(**img);
	}
	else
	{
		impl_->io.send_string("buffer", ZMQ_SNDMORE);
		impl_->io.send_string(boost::get<const std::string>(data));
	}

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}
	// else ok, no return value
}

void net_context::set_input_filter(const std::string &buffer, size_t channel, GLint minFilter)
{
	impl_->io.send_string("context", ZMQ_SNDMORE);
	impl_->io.send_string(id(), ZMQ_SNDMORE);
	impl_->io.send_string("set_input_filter", ZMQ_SNDMORE);

	// Send input specification
	impl_->io.send_string(buffer, ZMQ_SNDMORE);
	impl_->io.send_data<uint8_t>(channel, ZMQ_SNDMORE);

	// Send filter
	impl_->io.send_data<int32_t>(minFilter);

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}
	// else ok, no return value
}

void net_context::reset_input(const std::string &buffer, size_t channel)
{
	impl_->io.send_string("context", ZMQ_SNDMORE);
	impl_->io.send_string(id(), ZMQ_SNDMORE);
	impl_->io.send_string("reset_input", ZMQ_SNDMORE);

	// Send input specification
	impl_->io.send_string(buffer, ZMQ_SNDMORE);
	impl_->io.send_data<uint8_t>(channel);

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}
	// else ok, no return value
}

net_host::net_host(const std::string &target)
	: impl_(new net_host_impl(target))
{
}

net_host::~net_host()
{
	delete impl_;
}

void net_host::allocate()
{
	impl_->connect();
}

core::image net_host::render(const std::string &id, boost::optional<int> frame, size_t width, size_t height,
							 const std::array<float, 4> &mouse, GLenum format)
{
	int act_frame = frame.get_value_or(std::numeric_limits<int>::min());
	impl_->log->info("render id: {} frame: {} width: {} height: {}", id, act_frame, width, height);

	impl_->io.send_string("render", ZMQ_SNDMORE);

	impl_->io.send_string(id, ZMQ_SNDMORE);
	impl_->io.send_data<int32_t>(act_frame, ZMQ_SNDMORE);
	impl_->io.send_data<uint32_t>(width, ZMQ_SNDMORE);
	impl_->io.send_data<uint32_t>(height, ZMQ_SNDMORE);
	impl_->io.send_data_noout(mouse, ZMQ_SNDMORE);
	impl_->io.send_data<int32_t>(format);
	
	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}

	core::image result;

	// Get frame timing
	impl_->io.recv_data(result.frame_timing);

	// Get contents
	impl_->io.recv_data_noout(result);

	return result;
}

void net_host::reset(const std::string &id)
{
	impl_->log->info("reset id: {}", id);

	impl_->io.send_string("reset", ZMQ_SNDMORE);
	impl_->io.send_string(id);

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}
}

std::string net_host::create_local(const std::vector<std::pair<std::string, std::string>> &bufferSources)
{
	impl_->log->info("create_local sources: {}", bufferSources.size());

	impl_->io.send_string("create_local", ZMQ_SNDMORE);
	impl_->io.send_data<uint32_t>(bufferSources.size(), ZMQ_SNDMORE);

	for (auto pair : bufferSources)
	{
		impl_->io.send_string(pair.first, ZMQ_SNDMORE);
		impl_->io.send_string(pair.second, ZMQ_SNDMORE);
	}

	impl_->io.send_empty();

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());
	auto extra(impl_->io.recv_string());

	if (status.compare("OK") == 0)
	{
		return extra;
	}
	else
	{
		throw std::runtime_error(extra);
	}
}

std::shared_ptr<core::basic_context> net_host::get_context(const std::string &id)
{
	impl_->io.send_string("get_context", ZMQ_SNDMORE);
	impl_->io.send_string(id);

	impl_->io.recv_wait();

	auto status(impl_->io.recv_string());

	if (status.compare("ERROR") == 0)
	{
		throw std::runtime_error(impl_->io.recv_string());
	}

	return std::make_shared<net_context>(id, impl_);
}

