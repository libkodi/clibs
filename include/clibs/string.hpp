#ifndef _CLIBS_STRING_H_
#define _CLIBS_STRING_H_ 1

#include <iostream>
#include <sstream>

namespace clibs {
	template<typename T>
	std::string str_str(T value) {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}
}

#endif