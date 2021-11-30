#include <iostream>
#include "clibs/net/socket.hpp"

using namespace clibs;

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	net::socket_winsock_init(2, 2);
#endif

	net::socket_t sock;

	net::socket_new(AF_INET, SOCK_STREAM, 0, &sock);

	net::socket_connect(&sock, "127.0.0.1", 1234);
	net::socket_send(&sock, "3333333", 7);
	char buff[1024];
	int len;
	len = net::socket_recv(&sock, buff, 1024);
	buff[len] = '\0';
	std::cout << buff << std::endl;

	net::socket_shutdown(&sock, SHUT_RDWR);

#ifdef _WIN32
	net::socket_winsock_free();
#endif

	return 0;
}