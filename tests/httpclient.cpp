#include <iostream>
#include "clibs/net/http/httpclient.hpp"

int main(int argc, char const *argv[])
{
#ifdef _WIN32
	clibs::net::socket_winsock_init(2, 2);
#endif
	clibs::net::socket_ssl_init();

	clibs::net::http::CHttpClient client;

	std::string url = "https://www.baidu.com";
	// std::string url = "https://www.cnblogs.com/lnlvinso/p/11160827.html";
	// std::string url = "https://www.google.com";
	// std::string url = "http://localhost:1234";

	if (!client.set_url(url)) {
		std::cout << client.error() << std::endl;
		exit(1);
	}

	client.set_method("GET");
	
	if (!client.connect()) {
		std::cout << client.error() << std::endl;
		client.close();
		exit(1);
	}
	
	if (client.get_response()) {
		std::string res = client.read();
		std::cout << res << std::endl;
	}

	std::cout << client.get_status_code() << std::endl;
	std::cout << client.get_status_message() << std::endl;

	client.close();

#ifdef _WIN32
	clibs::net::socket_winsock_free();
#endif
	clibs::net::socket_ssl_free();
	return 0;
}