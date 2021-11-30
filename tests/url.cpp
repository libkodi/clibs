#include <iostream>
#include <exception>
#include "clibs/net/url.hpp"

int main(int argc, char const *argv[])
{
	clibs::net::url_t url;
	clibs::net::url_parse(&url, "https://localhost/test");

	std::cout << url.protocol << std::endl;
	std::cout << url.host << std::endl;
	std::cout << url.port << std::endl;
	std::cout << url.path << std::endl;

	return 0;
}