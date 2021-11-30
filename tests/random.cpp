#include <iostream>
#include "clibs/random.hpp"

int main(int argc, char const *argv[])
{
    clibs::CRandom r;

    std::cout << r.next() << std::endl;
    std::cout << r.next() << std::endl;
    std::cout << r.next() << std::endl;

    std::cout << r.next_int(10, 100) << std::endl;
    std::cout << r.next_int(1, 10) << std::endl;
    std::cout << r.next_int(1999999, 200000000) << std::endl;

    std::cout << r.next_float(0.0, 1.0) << std::endl;
    std::cout << r.next_float(3.0, 9.0) << std::endl;
    std::cout << r.next_float(100.0, 1000.0) << std::endl;
    return 0;
}