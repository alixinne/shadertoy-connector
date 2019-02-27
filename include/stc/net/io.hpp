#ifndef _STC_NET_IO_HPP_
#define _STC_NET_IO_HPP_

#include <zmq.hpp>

#include <shadertoy/spdlog/spdlog.h>

#include <shadertoy/spdlog/fmt/ostr.h>

#include "stc/core/image.hpp"

namespace stc
{
namespace net
{

class io
{
	std::shared_ptr<spdlog::logger> &log_;
	zmq::socket_t &socket_;

public:
	io(std::shared_ptr<spdlog::logger> &log, zmq::socket_t &socket);

	template <typename T>
	void send_data(const T &request, int flags = 0)
	{
		log_->debug("send({}): {}", sizeof(T), request);
		socket_.send(&request, sizeof(T), flags);
	}

	template <typename T>
	void send_data_noout(const T &request, int flags = 0)
	{
		log_->debug("send({}): <output suppressed>", sizeof(T));
		socket_.send(&request, sizeof(T), flags);
	}

	template <typename T>
	void send_buf(const std::vector<T> &t, int flags = 0)
	{
		size_t bytes = sizeof(T) * t.size();
		log_->debug("send({}): <output suppressed>", bytes);
		socket_.send(t.data(), bytes, flags);
	}

	void send_empty(int flags = 0);

	void send_string(const std::string &str, int flags = 0);

	bool recv_wait(int timeout = -1);

	template <typename T>
	void recv_data(T &response, int flags = 0)
	{
		socket_.recv(&response, sizeof(T), flags);
		log_->debug("recv({}): {}", sizeof(T), response);
	}

	template <typename T>
	void recv_data_noout(T &response, int flags = 0)
	{
		socket_.recv(&response, sizeof(T), flags);
		log_->debug("recv({}): <output suppressed>", sizeof(T));
	}

	template <typename T>
	T recv_data(int flags = 0)
	{
		T result;
		recv_data(result, flags);
		return result;
	}

	template <typename T>
	T recv_data_noout(int flags = 0)
	{
		T result;
		recv_data_noout(result, flags);
		return result;
	}

	template <typename T>
	void recv_buf(std::vector<T> &t, int flags = 0)
	{
		size_t bytes = t.size() * sizeof(T);
		size_t rcv = socket_.recv(t.data(), bytes, flags);
		assert(rcv == bytes);
		log_->debug("recv({}): <output suppressed>", rcv);
	}

	void recv_empty(int flags = 0);

	std::string recv_string(int flags = 0);
};

template <>
void io::send_data_noout<core::image>(const core::image &img, int flags);

template <>
void io::recv_data_noout<core::image>(core::image &img, int flags);
}
}

#endif /* _STC_NET_IO_HPP_ */
