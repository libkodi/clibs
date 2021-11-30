#include <iostream>
#include "clibs/io/select.hpp"

using namespace clibs::io;

int main(int argc, char const *argv[])
{
    select_t s, s1;

    select_init(&s);
    select_append(&s, 1, ST_READ | ST_WRITE);

    std::cout << select_contains(&s, 1, ST_READ) << std::endl;
    std::cout << select_contains(&s, 1, ST_WRITE) << std::endl;
    std::cout << select_contains(&s, 1, ST_EXCEPT) << std::endl;

    s1 = s;

    select_remove(&s1, 1, ST_WRITE | ST_READ);
    // select_append(&s1, 1, ST_EXCEPT);

    std::cout << select_contains(&s1, 1, ST_READ) << std::endl;
    std::cout << select_contains(&s1, 1, ST_WRITE) << std::endl;
    std::cout << select_contains(&s1, 1, ST_EXCEPT) << std::endl;

    std::cout << select_contains(&s, 1, ST_READ) << std::endl;
    std::cout << select_contains(&s, 1, ST_WRITE) << std::endl;
    std::cout << select_contains(&s, 1, ST_EXCEPT) << std::endl;

    std::cout << s1.max_sockfd << std::endl;

    return 0;
}