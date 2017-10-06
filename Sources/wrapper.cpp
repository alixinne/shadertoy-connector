#include "wrapper.hpp"

#include "mathlink.h"

// Failure callback
void st_fail(const char *msgname, const char *arg)
{
	MLPutFunction(stdlink, "CompoundExpression", 2);
	MLPutFunction(stdlink, "Message", 2);
	MLPutFunction(stdlink, "MessageName", 2);
	MLPutSymbol(stdlink, "Shadertoy");
	MLPutString(stdlink, msgname);
	MLPutString(stdlink, arg);
	MLPutSymbol(stdlink, "$Failed");
}

template<>
void st_wrapper_exec(std::function<void(void)> &&fun)
{
	st_wrapper_exec<bool>(std::function<bool(void)>([&fun]() {
		fun();
		return true;
	}));
}
