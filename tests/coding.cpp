#include <iostream>
#include <cerrno>
#include "clibs/iconv.hpp"

extern int errno;

int main(int argc, char const *argv[])
{
    std::string str = "iconv是GNU的一个库和软件，它在LGPL3或GPL3下发布，功能是进行字符编码集之间的转换，它支持大量的字符集，并且是一个可移植的库和程序。libiconv库已经非常简单了，然而由于它过于简单，使得它的一个接口函数功能太多，这让人们想到C标准库那个功能太多而难以使用的“realloc”函数，所有有许多决定写简单的封装。当然libiconv是一个C语言库，对习惯C++的编程风格的人们可以有一些不习惯了，所以我们在这里也决定写一个封装。iconv是GNU的一个库和软件，它在LGPL3或GPL3下发布，功能是进行字符编码集之间的转换，它支持大量的字符集，并且是一个可移植的库和程序。libiconv库已经非常简单了，然而由于它过于简单，使得它的一个接口函数功能太多，这让人们想到C标准库那个功能太多而难以使用的“realloc”函数，所有有许多决定写简单的封装。当然libiconv是一个C语言库，对习惯C++的编程风格的人们可以有一些不习惯了，所以我们在这里也决定写一个封装。";

    clibs::iconv_result_t res, res1;

    clibs::iconv_convert("utf-8", "gbk", str.c_str(), str.length(), &res);
    std::cout << res.data << std::endl;
    std::cout << res.length << std::endl;
    clibs::iconv_convert("gbk", "utf-8", res.data, res.length, &res1);
    std::cout << res1.data << std::endl;
    std::cout << res1.length << std::endl;

    clibs::iconv_free_result(&res);
    clibs::iconv_free_result(&res1);

    return 0;
}