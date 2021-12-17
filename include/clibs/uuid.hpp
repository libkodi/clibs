#ifndef _CLIBS_UUID_H_
#define _CLIBS_UUID_H_ 1

#include <iostream>
#include <cstring>
#include <sstream>

#include <uuid/uuid.h>

namespace clibs {
    std::string uuid() {
        uuid_t uid;
        uuid_generate(uid);
        char temp[2];
        std::string uuid_str;

        for (int i = 0; i < 16; i ++) {
            memset(temp, 0, 2);
            sprintf(temp, "%02x", uid[i]);
            uuid_str += temp;

            if (i == 3 || i == 5 || i == 7 || i == 9) {
                uuid_str += "-";
            }
        }

        return uuid_str;
    }
}

#endif