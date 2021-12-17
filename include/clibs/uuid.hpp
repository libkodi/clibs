#ifndef _CLIBS_UUID_H_
#define _CLIBS_UUID_H_ 1

#include <iostream>
#include <cstring>
#include <sstream>

#ifdef _WIN32
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#else
#include <uuid/uuid.h>
#endif

namespace clibs {
    std::string uuid() {
        std::string res;
        char uuid_cstr[37];
#ifdef _WIN32
        GUID uuid;
        CoCreateGuid(&uuid);
        sprintf(uuid_cstr, "%08x-", uuid.Data1);
        sprintf(uuid_cstr + 9, "%04x-", uuid.Data2);
        sprintf(uuid_cstr + 14, "%04x-", uuid.Data3);
        sprintf(uuid_cstr + 19, "%02x", uuid.Data4[0]);
        sprintf(uuid_cstr + 21, "%02x-", uuid.Data4[1]);
        sprintf(uuid_cstr + 24, "%02x", uuid.Data4[2]);
        sprintf(uuid_cstr + 26, "%02x", uuid.Data4[3]);
        sprintf(uuid_cstr + 28, "%02x", uuid.Data4[4]);
        sprintf(uuid_cstr + 30, "%02x", uuid.Data4[5]);
        sprintf(uuid_cstr + 32, "%02x", uuid.Data4[6]);
        sprintf(uuid_cstr + 34, "%02x", uuid.Data4[7]);
        uuid_cstr[36] = '\0';
#else
        uuid_t uid;
        uuid_generate(uid);
        uuid_unparse(uid, uuid_cstr);
#endif
        return std::string(uuid_cstr);        
    }
}

#endif