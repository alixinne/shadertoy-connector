#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/all.hpp>

#include "context.hpp"
#include "host.hpp"

#include <oct.h>

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
	catch (oglplus::ProgramBuildError &pbe)
	{
		std::stringstream ss;
		ss << "Program build error: " << pbe.what() << " [" << pbe.SourceFile() << ":"
		   << pbe.SourceLine() << "] " << pbe.Log();
		octave_stdout << ss.str();
	}
	catch (oglplus::Error &err)
	{
		std::stringstream ss;
		ss << "Error: " << err.what() << " [" << err.SourceFile() << ":" << err.SourceLine() << "] "
		   << err.Log();
		octave_stdout << ss.str();
	}
	catch (std::runtime_error &ex)
	{
		octave_stdout << ex.what();
	}

	return TRet();
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

DEFUN_DLD (st_render, args, nargout,
		   "st_render('id', [frame, [width, [height, [mouse]]]]) renders a Shadertoy as an image")
{
	if (args.length() < 1 || args.length() > 5)
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
				frame = boost::optional<int>(args(1).int_value());

			int width = 640;
			if (args.length() >= 3)
				width = args(2).int_value();

			int height = 360;
			if (args.length() >= 4)
				height = args(3).int_value();

			float mouse[4] = { 0.f, 0.f, 0.f, 0.f };
			if (args.length() >= 5)
			{
				auto mouse_arg(args(5).array_value());
				auto mouse_arg_dim(mouse_arg.dims());
				const char err[] = "The mouse argument must be a 2 or 4 element 1D double array.";

				if (mouse_arg_dim.length() != 1)
					throw std::runtime_error(err);

				if (mouse_arg_dim(0) != 2 && mouse_arg_dim(1) != 4)
					throw std::runtime_error(err);

				for (int i = 0; i < mouse_arg_dim(0); ++i)
					mouse[i] = static_cast<float>(mouse_arg(i));
			}

			// Render the image
			auto image(st_host().Render(shaderId, frame, width, height, mouse));

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
