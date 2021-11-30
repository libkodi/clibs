#include <iostream>
#include "clibs/net/socket.hpp"

using namespace clibs;

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	net::socket_winsock_init(2, 2);
#endif

	net::socket_t sock;
	net::socket_new(AF_INET, SOCK_DGRAM, 0, &sock);

	net::socket_set_address(&sock, "127.0.0.1", 1234);
	net::socket_sendto(&sock, "3333333", 7, &sock);
	char buff[1024];
	int len;
	
	net::socket_t nsock;

	len = net::socket_recvfrom(&sock, buff, 1024, &nsock);
	buff[len] = '\0';
	std::cout << buff << std::endl;

	net::socket_shutdown(&sock, SHUT_RDWR);

#ifdef _WIN32
	net::socket_winsock_free();
#endif

	return 0;
}