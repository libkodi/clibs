#ifndef _CLIBS_EXCEPTION_H_
#define _CLIBS_EXCEPTION_H_ 1

// #include <iostream>
// #include <exception>
#include <cerrno>
#include <cstring>

extern int errno;

namespace clibs {
	/**
	 * std::exception的扩展封装, 添加错误描述支持
	 */
	// class CBaseException : public std::exception {
	// 	protected:
	// 		std::string message;

	// 	public:
	// 		CBaseException() {};

	// 		CBaseException(std::string errmsg) {
	// 			message = errmsg;
	// 		}

	// 		const char* what() const throw() {
	// 			return message.c_str();
	// 		}
	// };

	/**
	 * 返回当前errno指向的错误描述
	 */
	std::string errstr() {
		return strerror(errno);
	}
}

#endif