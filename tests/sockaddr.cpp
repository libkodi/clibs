#include <iostream>
#include "clibs/net/socket.hpp"

using namespace clibs;

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	net::socket_winsock_init(2, 2);
#endif
	net::socket_t sock;

	net::socket_set_address(&sock, "www.baidu.com", 80);	

	std::cout << net::socket_get_port(&sock) << std::endl;
	std::cout << net::socket_get_ip(&sock) << std::endl;
	std::cout << SHUT_RD << std::endl;
	std::cout << SHUT_WR << std::endl;
	std::cout << SHUT_RDWR << std::endl;
#ifdef _WIN32
	net::socket_winsock_free();
#endif
	
	return 0;
}