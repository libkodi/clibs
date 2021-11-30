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

	net::socket_new(AF_INET, SOCK_STREAM, 0, &sock);

	net::socket_bind(&sock, "127.0.0.1", 1234);
	net::socket_listen(&sock, 5);

	std::cout << "wait..." << std::endl;

	net::socket_t nsock;
	net::socket_accept(&sock, &nsock);

	char buff[1024];
	int len;

	len = net::socket_recv(&nsock, buff, 1024);
	buff[len] = '\0';

	std::cout << buff << std::endl;
	net::socket_send(&nsock, buff, strlen(buff), 0);
	net::socket_shutdown(&nsock, SHUT_RDWR);

	net::socket_shutdown(&sock, SHUT_RDWR);

#ifdef _WIN32
	net::socket_winsock_free();
#endif

	return 0;
}