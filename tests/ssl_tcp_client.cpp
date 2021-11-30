#include <iostream>
#include <cstring>
#include "clibs/net/sslsocket.hpp"

using namespace clibs;

int main(int argc, char const *argv[])
{
	net::socket_ssl_init();

#ifdef OS_WIN32
	net::socket_winsock_init(2, 2);
#endif
	net::socket_t sock;
	net::ssl_socket_t ssock;

	net::socket_new(AF_INET, SOCK_STREAM, 0, &sock);

	net::socket_connect(&sock, "www.baidu.com", 443);

	net::socket_ssl_new(&ssock);
	
	net::socket_ssl_bind(&ssock, &sock);

	net::socket_ssl_connect(&ssock);

	char *con = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n";

	net::socket_ssl_send(&ssock, con, strlen(con));

	char buff[4096];
	int len;

	len = net::socket_ssl_recv_must(&ssock, buff, 4096);
	buff[len] = '\0';

	std::cout << buff << std::endl;

	net::socket_ssl_close(&ssock);
	net::socket_shutdown(&sock, SHUT_RDWR);
	net::socket_close(&sock);

	net::socket_ssl_free();
	
#ifdef OS_WIN32
	net::socket_winsock_free();
#endif
	return 0;
}