#ifndef _CLIBS_UUID_H_
#define _CLIBS_UUID_H_ 1

#include <iostream>
#include <cstring>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <uuid/uuid.h>
#endif

namespace clibs {
    std::string uuid() {
        std::string res;
#ifdef _WIN32
        UuidCreate(&uuid);
        char* uuid_cstr = nullptr;
        UuidToStringA(&uuid, reinterpret_cast<RPC_CSTR*>(&uuid_cstr));
        res = std::string(uuid_cstr);
        RpcStringFreeA(reinterpret_cast<RPC_CSTR*>(&uuid_cstr));
#else
        uuid_t uid;
        char uuid_cstr[36];
        uuid_generate(uid);
        uuid_unparse(uid, uuid_cstr);
        res = std::string(uuid_cstr);
#endif
        
        return res;
    }
}

#endif