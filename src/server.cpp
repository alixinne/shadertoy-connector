#include <sstream>

#include <boost/variant.hpp>

#include <zmq.hpp>

#include <shadertoy/spdlog/spdlog.h>
#include <shadertoy/spdlog/fmt/ostr.h>

#include "server.hpp"
#include "basic_context.hpp"
#include "gl_host.hpp"
#include "net_io.hpp"

class server_impl
{
	const std::string bind_address_;
	zmq::context_t context_;
	zmq::socket_t socket_;

	gl_host rendering_context_;
	std::shared_ptr<spdlog::logger> log_;

	net_io io_;

	void handle_context_set_input(const std::shared_ptr<basic_context> &context)
	{
		// Get input specification
		auto buffer(io_.recv_string());
		auto channel(io_.recv_data<size_t>());

		// Get input type
		auto input_type(io_.recv_string());
		boost::variant<std::string, std::shared_ptr<StImage>> value;

		// Get input data
		if (input_type.compare("image") == 0)
		{
			auto img(std::make_shared<StImage>());
			io_.recv_data_noout(*img);
			value = img;
		}
		else if (input_type.compare("buffer") == 0)
		{
			value = io_.recv_string();
		}
		else
		{
			std::stringstream ss;
			ss << "Unknown input type: " << input_type;
			throw std::runtime_error(ss.str());
		}

		// Apply
		context->set_input(buffer, channel, value);

		io_.send_string("OK");
	}

	void handle_context_set_input_filter(const std::shared_ptr<basic_context> &context)
	{
		// Get input specification
		auto buffer(io_.recv_string());
		auto channel(io_.recv_data<size_t>());

		// Get input filter
		auto filter(io_.recv_data<GLenum>());

		// Apply
		context->set_input_filter(buffer, channel, filter);

		io_.send_string("OK");
	}

	void handle_context_reset_input(const std::shared_ptr<basic_context> &context)
	{
		// Get input specification
		auto buffer(io_.recv_string());
		auto channel(io_.recv_data<size_t>());

		// Apply
		context->reset_input(buffer, channel);

		io_.send_string("OK");
	}

	void handle_render()
	{
		auto id(io_.recv_string());
		auto frame(io_.recv_data<int>());
		auto width(io_.recv_data<size_t>());
		auto height(io_.recv_data<size_t>());
		auto mouse(io_.recv_data_noout<std::array<float, 4>>());
		auto format(io_.recv_data<GLenum>());

		try
		{
			auto img(rendering_context_.render(id, frame, width, height, mouse, format));

			log_->info("Rendered frame {} for {}", frame, id);
			io_.send_string("OK", ZMQ_SNDMORE);

			// Send frame timing
			io_.send_data(img.frameTiming, ZMQ_SNDMORE);

			// Send image
			io_.send_data_noout(img);
		}
		catch (std::exception &ex)
		{
			log_->warn("Could not render context {}: {}", id, ex.what());

			io_.send_string("ERROR", ZMQ_SNDMORE);
			io_.send_string(ex.what());
		}
	}

	void handle_reset()
	{
		auto id(io_.recv_string());

		try
		{
			rendering_context_.reset(id);

			log_->info("Reset context {}", id);
			io_.send_string("OK");
		}
		catch (std::exception &ex)
		{
			log_->warn("Could not reset context {}: {}", id, ex.what());

			io_.send_string("ERROR", ZMQ_SNDMORE);
			io_.send_string(ex.what());
		}
	}

	void handle_create_local()
	{
		auto part_cnt(io_.recv_data<int>());

		std::vector<std::pair<std::string, std::string>> buffer_sources;

		for (int i = 0; i < part_cnt; ++i)
		{
			std::string name(io_.recv_string()),
						source(io_.recv_string());

			log_->info("create_local: {} => {}", name, source);

			buffer_sources.emplace_back(name, source);
		}

		io_.recv_empty();

		try
		{
			auto context_id(rendering_context_.create_local(buffer_sources));

			log_->info("Created local context {}", context_id);

			io_.send_string("OK", ZMQ_SNDMORE);
			io_.send_string(context_id);
		}
		catch (std::exception &ex)
		{
			log_->warn("Could not create local context: {}", ex.what());

			io_.send_string("ERROR", ZMQ_SNDMORE);
			io_.send_string(ex.what());
		}
	}

	void handle_get_context()
	{
		auto id(io_.recv_string());

		try
		{
			auto context(rendering_context_.get_context(id));

			log_->info("Got context {} at {}", id, (void*)context.get());
			io_.send_string("OK");
		}
		catch (std::exception &ex)
		{
			log_->warn("Could not get context {}: {}", id, ex.what());

			io_.send_string("ERROR", ZMQ_SNDMORE);
			io_.send_string(ex.what());
		}
	}

	void handle_context()
	{
		auto id(io_.recv_string());
		auto method(io_.recv_string());

		std::shared_ptr<basic_context> context;

		try
		{
			auto context = rendering_context_.get_context(id);
			log_->info("Found context {}", id);

			if (method.compare("set_input") == 0)
			{
				handle_context_set_input(context);
			}
			else if (method.compare("set_input_filter") == 0)
			{
				handle_context_set_input_filter(context);
			}
			else if (method.compare("reset_input") == 0)
			{
				handle_context_reset_input(context);
			}
			else
			{
				std::stringstream ss;
				ss << "Invalid context method: '" << method << "'";
				throw std::runtime_error(ss.str());
			}
		}
		catch (std::exception &ex)
		{
			log_->warn("Exception occurred in handle_context, clearing messages");
			log_->warn("Exception: {}", ex.what());

			// Dump pending messages
			int value;
			do
			{
				size_t opt = sizeof(int);
				socket_.getsockopt(ZMQ_RCVMORE, &value, &opt);

				if (value)
				{
					zmq::message_t discard;
					socket_.recv(&discard);
				}
			}
			while (value);

			io_.send_string("ERROR", ZMQ_SNDMORE);
			io_.send_string(ex.what());
		}
	}

public:
	server_impl(const std::string bind_address)
		: bind_address_(bind_address),
		context_(1),
		socket_(context_, ZMQ_REP),
		rendering_context_(),
		log_(spdlog::stderr_color_mt("shadertoy-server")),
		io_(log_, socket_)
	{
	}

	void run()
	{
		log_->info("Creating OpenGL context");
		rendering_context_.allocate();

		log_->info("Binding to {}", bind_address_);
		socket_.bind(bind_address_);

		while (true)
		{
			// Recv request
			io_.recv_wait();
			auto request_name(io_.recv_string());

			log_->info("Got request header: '{}'", request_name);

			if (request_name.compare("render") == 0)
			{
				handle_render();
			}
			else if (request_name.compare("reset") == 0)
			{
				handle_reset();
			}
			else if (request_name.compare("create_local") == 0)
			{
				handle_create_local();
			}
			else if (request_name.compare("get_context") == 0)
			{
				handle_get_context();
			}
			else if (request_name.compare("context") == 0)
			{
				handle_context();
			}
			else
			{
				log_->error("Unknown request name");
				io_.send_string("ERROR", ZMQ_SNDMORE);
				io_.send_string("Unknown request name");
			}
		}
	}
};

server::server(const std::string &bind_address)
	: impl_(new server_impl(bind_address))
{
}

server::~server()
{
	delete impl_;
}

void server::run()
{
	impl_->run();
}
