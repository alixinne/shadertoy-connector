#include "stc/net/io.hpp"

using namespace stc;
using namespace stc::net;

io::io(std::shared_ptr<spdlog::logger> &log, zmq::socket_t &socket)
	: log_(log),
	socket_(socket)
{
}

void io::send_empty(int flags)
{
	log_->debug("send(0)");

	zmq::message_t msg;
	socket_.send(msg, flags);
}

void io::send_string(const std::string &str, int flags)
{
	log_->debug("send({}): '{}'", str.size(), str);

	zmq::message_t msg(str.size());
	memcpy(msg.data(), str.c_str(), str.size());
	socket_.send(msg, flags);
}

bool io::recv_wait(int timeout)
{
	zmq::pollitem_t items[] = {
		{ static_cast<void*>(socket_), 0, ZMQ_POLLIN, 0 }
	};

	try
	{
		zmq::poll(&items[0], sizeof(items)/sizeof(items[0]), timeout);
	}
	catch (zmq::error_t &e)
	{
		log_->warn("zmq::poll: {}", e.what());
		return false;
	}

	if (items[0].revents & ZMQ_POLLIN)
		return true;

	return false;
}

void io::recv_empty(int flags)
{
	zmq::message_t msg;
	socket_.recv(&msg, flags);
	assert(msg.size() == 0);
	log_->debug("recv(0)");
}

std::string io::recv_string(int flags)
{
	zmq::message_t msg;
	socket_.recv(&msg, flags);

	std::string result(reinterpret_cast<char*>(msg.data()),
					   reinterpret_cast<char*>(msg.data()) + msg.size());

	log_->debug("recv({}): '{}'", result.size(), result);

	return result;
}

template <>
void io::send_data_noout<core::image>(const core::image &img, int flags)
{
	// Send image dimensions
	send_data_noout(img.dims, ZMQ_SNDMORE | flags);

	// Send buffer
	send_buf(*img.data, flags);
}

template <>
void io::recv_data_noout<core::image>(core::image &img, int flags)
{
	// Get image dimensions
	recv_data_noout(img.dims, flags);

	// Allocate storage
	img.alloc();

	// Get data
	recv_buf(*img.data, flags);
}
