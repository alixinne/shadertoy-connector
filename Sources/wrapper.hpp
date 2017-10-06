#ifndef _WRAPPER_HPP_
#define _WRAPPER_HPP_

#include <stdexcept>
#include <functional>
#include <sstream>

#include <GL/glew.h>
#include <oglplus/all.hpp>

extern void st_fail(const char *msgname, const char *arg);

template<typename TRet>
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
		st_fail("glerr", ss.str().c_str());
	}
	catch (oglplus::Error &err)
	{
		std::stringstream ss;
		ss << "Error: " << err.what() << " [" << err.SourceFile() << ":" << err.SourceLine() << "] "
		   << err.Log();
		st_fail("glerr", ss.str().c_str());
	}
	catch (std::runtime_error &ex)
	{
		st_fail("err", ex.what());
	}

	return TRet();
}

template<>
void st_wrapper_exec(std::function<void(void)> &&fun);

#endif /* _WRAPPER_HPP_ */
