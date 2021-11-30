#include <iostream>
#include "clibs/array.hpp"
#include <typeinfo>

int main(int argc, char const *argv[])
{
    const void* str = "aaddd";

    clibs::foreach_args([](size_t index, auto arg) -> bool {
        std::cout << typeid(arg).name() << "[" << index << "]" << ":" << arg << std::endl;
        return true;
    }, 1, 5, 'a', "aaa", str);
    return 0;
}