#include <iostream>
#include <clibs/uuid.hpp>

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 20; i ++) {
        std::cout << clibs::uuid() << std::endl;
    }
    return 0;
}