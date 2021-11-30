#include <iostream>
#include "clibs/array.hpp"

int main(int argc, char const *argv[])
{
    std::vector<std::string> arr_str;
    arr_str.push_back("abc");
    arr_str.push_back("kdkd");
    arr_str.push_back("def");
    arr_str.push_back("mnz");
    int arr_int[10] = { 1, 2, 5, 9, 0, 4, 8, 7, 3, 6 };

    clibs::array_foreach(&arr_str, [](size_t index, std::vector<std::string>::iterator it) -> bool {
        std::cout << *it << std::endl;
        return true;
    });

    std::string def = "def";

    std::cout << "def-index: " << clibs::array_index(&arr_str, def) << std::endl;

    clibs::array_foreach(arr_int, 10, [](size_t index, int* val) -> bool {
        std::cout << *val << std::endl;
        return true;
    });

    std::cout << "5-index: " << clibs::array_index(arr_int, 10, 5) << std::endl;

    return 0;
}