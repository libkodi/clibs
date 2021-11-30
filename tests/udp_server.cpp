#include <iostream>
#include <cstring>
#include "clibs/net/socket.hpp"

using namespace clibs;

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	net::socket_winsock_init(2, 2);
#endif

	net::socket_t sock;
	net::socket_new(AF_INET, SOCK_DGRAM, 0, &sock);

	net::socket_bind(&sock, "127.0.0.1", 1234);

	std::cout << "wait..." << std::endl;

	char buff[1024];
	int len;
	net::socket_t client;

	len = socket_recvfrom(&sock, buff, 1024, &client);
	buff[len] = '\0';

	std::cout << buff << std::endl;
	std::cout << net::socket_get_ip(&client) << std::endl;
	std::cout << net::socket_get_port(&client) << std::endl;
	net::socket_sendto(&sock, buff, strlen(buff), &client);

	net::socket_shutdown(&sock, SHUT_RDWR);

#ifdef _WIN32
	net::socket_winsock_free();
#endif
	return 0;
}