#include <functional>

#include <epoxy/gl.h>
#include <GLFW/glfw3.h>

#include "context.hpp"
#include "host.hpp"

#include <oct.h>
#include <parse.h>

Host host;
bool host_initialized;

Host &st_host()
{
	if (!host_initialized)
	{
		host.Allocate();
		host_initialized = true;
	}

	return host;
}

template <typename TRet>
TRet st_wrapper_exec(std::function<TRet(void)> &&fun)
{
	try
	{
		return fun();
	}
	catch (shadertoy::OpenGL::ShaderCompilationError &ex)
	{
		octave_stdout << "Shader compilation error: " << ex.what();
	}
	catch (shadertoy::OpenGL::ProgramLinkError &ex)
	{
		octave_stdout << "Program link error: " << ex.what();
	}
	catch (shadertoy::OpenGL::OpenGLError &ex)
	{
		octave_stdout << "OpenGL error: " << ex.what();
	}
	catch (shadertoy::ShadertoyError &ex)
	{
		octave_stdout << "Shadertoy error: " << ex.what();
	}
	catch (std::runtime_error &ex)
	{
		octave_stdout << ex.what();
	}

	return TRet();
}

void oct_autoload(const char *fname)
{
	octave_value_list args;
	args(0) = std::string(fname);
	args(1) = std::string("shadertoy_octave.oct");

	feval("autoload", args);
}

DEFUN_DLD (shadertoy_octave, args, nargout,
		   "shadertoy_octave() initializes the shadertoy oct file")
{
	oct_autoload("st_render");
	oct_autoload("st_reset");
	oct_autoload("st_compile");
	oct_autoload("st_set_input");
	oct_autoload("st_set_input_filter");
	oct_autoload("st_reset_input");

	return octave_value();
}

DEFUN_DLD (st_render, args, nargout,
		   "st_render('id', [frame, [format, [width, [height, [mouse]]]]]) renders a Shadertoy as an image")
{
	if (args.length() < 1 || args.length() > 6)
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the shader id
			std::string shaderId(args(0).string_value());

			// Parse more arguments
			boost::optional<int> frame;
			if (args.length() >= 2)
			{
				int value(args(1).int_value());
				if (value >= 0)
					frame = boost::optional<int>(value);
			}

			GLenum format = GL_RGB;
			if (args.length() >= 3)
			{
				std::string formatName(args(2).string_value());

				if (formatName.compare("RGBA") == 0)
					format = GL_RGBA;
				else if (formatName.compare("RGB") == 0)
					format = GL_RGB;
				else if (formatName.compare("Luminance") == 0)
					format = GL_LUMINANCE;
				else
					throw std::runtime_error("Invalid format parameter");
			}

			int width = 640;
			if (args.length() >= 4)
				width = args(3).int_value();

			int height = 360;
			if (args.length() >= 5)
				height = args(4).int_value();

			float mouse[4] = { 0.f, 0.f, 0.f, 0.f };
			if (args.length() >= 6)
			{
				auto mouse_arg(args(5).array_value());
				auto mouse_arg_dim(mouse_arg.dims());

				if (mouse_arg_dim.length() != 2)
					throw std::runtime_error("The mouse argument must be a 1D double array.");

				if (mouse_arg_dim(1) != 2 && mouse_arg_dim(1) != 4)
					throw std::runtime_error("The mouse argument should have 2 or 4 elements.");

				for (int i = 0; i < mouse_arg_dim(1); ++i)
					mouse[i] = static_cast<float>(mouse_arg(0, i));
			}

			// Render the image
			auto image(st_host().Render(shaderId, frame, width, height, mouse, format));

			// Return the image
			auto dims(dim_vector(image->dims[0], image->dims[1], image->dims[2]));
			NDArray data(dims);

			// Need to copy from float* to double*
			for (size_t i = 0; i < image->dims[0]; ++i)
				for (size_t j = 0; j < image->dims[1]; ++j)
					for (size_t k = 0; k < image->dims[2]; ++k)
					{
						size_t idx = (i * image->dims[1] + j) * image->dims[2] + k;
						data(i, j, k) = static_cast<double>((*image->data)[idx]);
					}

			return octave_value(data);
		}));
	}
}

DEFUN_DLD (st_reset, args, nargout,
		   "st_reset('id') resets a context")
{
	if (args.length() != 1)
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the id
			std::string id(args(0).string_value());

			// Reset
			host.Reset(id);

			return octave_value();
		}));
	}
}

DEFUN_DLD (st_compile, args, nargout,
		   "st_compile('source') compiles the source of a program and returns its id for st_render")
{
	if (args.length() != 1)
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the source
			std::string shaderSource(args(0).string_value());

			// Compile
			auto id(st_host().CreateLocal(shaderSource));

			// Return value
			return octave_value(id);
		}));
	}
}

bool st_parse_input_name(const std::string &inputStr, std::string &buffer, int &channel)
{
	auto dotPos(inputStr.find('.'));
	if (dotPos == std::string::npos)
		throw std::runtime_error("Invalid input specification");

	buffer.assign(inputStr.begin(), inputStr.begin() + dotPos);
	std::string inputChannelStr(inputStr.begin() + dotPos + 1, inputStr.end());
	channel = atoi(inputChannelStr.c_str());

	if (channel < 0 || channel > 3)
		throw std::runtime_error("Invalid channel number");

	return true;
}

DEFUN_DLD (st_set_input, args, nargout,
			"st_set_input('id', 'image.0', matrix1[, 'image.1', matrix2[, ...]]])")
{
	if (args.length() < 3 || !(args.length() % 2 == 1))
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the context name
			std::string id(args(0).string_value());
			// Get the context
			auto context(host.GetContext(id));

			// Parse inputs
			for (int i = 1; i < args.length(); i += 2)
			{
				// Get the input name
				std::string inputName(args(i).string_value());
				// Get the input data
				NDArray img = args(i + 1).array_value();

				if (error_state)
					throw std::runtime_error("Failed to get the input specification");

				// Create the image structure
				int d = img.dims().length();
				if (d <= 1 || d > 3)
					throw std::runtime_error("Invalid number of dimensions for image array");

				StImage im;
				im.dims[0] = img.dim1();
				im.dims[1] = img.dim2();
				im.dims[2] = d == 3 ? img.dim3() : 1;
				im.data = std::make_shared<std::vector<float>>(im.dims[0] * im.dims[1] * im.dims[2]);

				// Copy data
				for (size_t i = 0; i < im.dims[0]; ++i)
					for (size_t j = 0; j < im.dims[1]; ++j)
						for (size_t k = 0; k < im.dims[2]; ++k)
						{
							size_t idx = (i * im.dims[1] + j) * im.dims[2] + k;
							if (d == 3)
								(*im.data)[idx] = static_cast<float>(img(i, j, k));
							else
								(*im.data)[idx] = static_cast<float>(img(i, j));
						}

				// Set input
				std::string bufferName;
				int channel;
				if (st_parse_input_name(inputName, bufferName, channel))
					context->setInput(bufferName, channel, im);
				else
					throw std::runtime_error("Failed to parse input name.");
			}

			return octave_value();
		}));
	}
}

DEFUN_DLD (st_set_input_filter, args, nargout,
			"st_set_input('id', 'image.0', 'linear'[, 'image.1', 'nearest'[, ...]]])")
{
	if (args.length() < 3 || !(args.length() % 2 == 1))
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the context name
			std::string id(args(0).string_value());
			// Get the context
			auto context(host.GetContext(id));

			// Parse inputs
			for (int i = 1; i < args.length(); i += 2)
			{
				// Get the input name
				std::string inputName(args(i).string_value());
				// Get the filter type
				std::string filterType(args(i + 1).string_value());

				if (error_state)
					throw std::runtime_error("Failed to get the input filter specification");

				// Parse filter type
				GLint filter;
				if (filterType.compare("nearest") == 0)
					filter = GL_NEAREST;
				else if (filterType.compare("linear") == 0)
					filter = GL_LINEAR;
				else if (filterType.compare("mipmap") == 0)
					filter = GL_LINEAR_MIPMAP_LINEAR;
				else
				{
					std::stringstream ss;
					ss << "Invalid filter type " << filterType << " for input "
						<< inputName;
					throw std::runtime_error(ss.str());
				}

				// Set input
				std::string bufferName;
				int channel;
				if (st_parse_input_name(inputName, bufferName, channel))
					context->setInputFilter(bufferName, channel, filter);
				else
					throw std::runtime_error("Failed to parse input name.");
			}

			return octave_value();
		}));
	}
}

DEFUN_DLD (st_reset_input, args, nargout,
			"st_reset_input('id', 'image.0'[, 'image.1'[, ...]]])")
{
	if (args.length() < 2)
	{
		print_usage();
		return octave_value();
	}
	else
	{
		return st_wrapper_exec(std::function<octave_value(void)>([&]() {
			// Get the context name
			std::string id(args(0).string_value());
			// Get the context
			auto context(host.GetContext(id));

			// Parse inputs
			for (int i = 1; i < args.length(); i++)
			{
				// Get the input name
				std::string inputName(args(i).string_value());

				if (error_state)
					throw std::runtime_error("Failed to get the input filter specification");

				// Set input
				std::string bufferName;
				int channel;
				if (st_parse_input_name(inputName, bufferName, channel))
					context->resetInput(bufferName, channel);
				else
					throw std::runtime_error("Failed to parse input name.");
			}

			return octave_value();
		}));
	}
}
