#include <iostream>
#include "clibs/regexp.hpp"

std::string cb(std::smatch match_t) {
	std::cout << match_t.str() << std::endl;
	return "pp";
}

int main(int argc, char const *argv[])
{
	std::string str = "Hello World!";

	clibs::CRegexp reg("l");

	if (clibs::test("^Hello", str)) {
		std::cout << "has hello" << std::endl;
	} else {
		std::cout << "not hello" << std::endl;
	}

	std::smatch m = reg.search(str);

	std::cout << m.size() << std::endl;
	std::cout << m.position(0) << std::endl;
	std::cout << m.position(1) << std::endl;
	std::cout << m.str() << std::endl;

	std::cout << clibs::match("Hello World!", str) << std::endl;

	clibs::CMatcher mc = reg.matches(str);

	while (mc.find()) {
		std::cout << mc.result().str() << std::endl;
	}

	std::cout << clibs::replace("l", "ll", "a") << std::endl;

	std::vector<std::string> arr = clibs::split("l", "alalalallllla");

	for (int i = 0, len = arr.size(); i < len; i ++) {
		std::cout << arr[i] << std::endl;
	}

	std::cout << clibs::replace("l", "alala", cb) << std::endl;

	return 0;
}