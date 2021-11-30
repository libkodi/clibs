#include <iostream>
#include "clibs/encrypt/sha1.hpp"

int main(int argc, char const *argv[])
{
    std::cout << clibs::encrypt::sha1_hex("abc", 3) << std::endl;
    return 0;
}