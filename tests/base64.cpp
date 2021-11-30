#include "clibs/encrypt/base64.hpp"

int main(int argc, char const *argv[])
{
    char* buff = "123";
    char outbuf[clibs::encrypt::base64_encode_size(strlen(buff))];;
    size_t outlen = 0;
    clibs::encrypt::base64_encode(buff, strlen(buff), outbuf, &outlen);

    printf("Encode: %s\n", outbuf);

    clibs::encrypt::base64_decode(outbuf, outlen, outbuf, &outlen);

    printf("Decode: %s\n", outbuf);

    return 0;
}