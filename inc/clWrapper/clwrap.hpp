#ifndef CLWRAP_HPP
#define CLWRAP_HPP

#include <CL/cl.hpp>
#include <stdarg.h>
#include <string>

namespace cl {
	std::string getErrMsg(int err);
}

#endif
