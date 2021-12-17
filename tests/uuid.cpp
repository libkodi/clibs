#include <iostream>
#include <clibs/uuid.hpp>

int main(int argc, char const *argv[])
{
    for (int i = 0; i < 10; i ++) {
        std::string res = clibs::uuid();
        std::cout << res << ":" << res.length() << std::endl;
    }

    return 0;
}