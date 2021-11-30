#include <iostream>
#include "clibs/string.hpp"

int main(int argc, char const *argv[])
{
	// std::string b = clibs::to_string(false);
	// std::string i = clibs::to_string(1);
	// std::string d = clibs::to_string(3.14);
	// std::string f = clibs::to_string('\44');

	// std::cout << i << std::endl;
	// std::cout << b << std::endl;
	// std::cout << d << std::endl;
	// std::cout << f << std::endl;
	// std::cout << clibs::concat(2, "abc", "ddd") << std::endl;

	// std::string aaa = "lll";
	// size_t b1 = 1233333;
	// float f1 = 3.14;

	// std::cout << clibs::format("%sabc%d%ld   %s %f", "abc", 1, b1, aaa.c_str(), f1) << std::endl;
	std::string res, res1;

	res += '\0';
	res += '1';
	res += '2';
	res += '3';

	char buff[4] = { '\0', 'a', 'b', 'c' };

	res1 = std::string(buff);

	res += res1;

	std::cout << res << std::endl;
	std::cout << res.length() << std::endl;

	return 0;
}