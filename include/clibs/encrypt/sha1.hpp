#ifndef _CLIBS_SHA1_H_
#define _CLIBS_SHA1_H_ 1
#include <iostream>
#include <sstream>
#include <openssl/sha.h>

namespace clibs {
    namespace encrypt {
        /**
         * 初始化上下文
         * @param  ctx 上下文
         * @return     trur/false
         */
        bool sha1_init(SHA_CTX* ctx) {
            return SHA1_Init(ctx);
        }

        /**
         * 附加加密文本
         * @param  ctx    上下文
         * @param  data   附加数据
         * @param  length 数据长度
         * @return        true/false
         */
        bool sha1_update(SHA_CTX* ctx, const char* data, size_t length) {
            return SHA1_Update(ctx, data, length);
        }

        /**
         * 获取加密结果
         * @param ctx    上下文
         * @param buffer 接收结果的内存
         */
        void sha1_digest(SHA_CTX* ctx, unsigned char* buffer) {
            SHA1_Final(buffer, ctx);
        }

        /**
         * 将结果转换为hex格式字符串
         * @param  ctx 上下文
         * @return     加密结果字符串
         */
        std::string sha1_hex_digest(SHA_CTX* ctx) {
            std::stringstream ss;
            unsigned char buffer[SHA_DIGEST_LENGTH];
            sha1_digest(ctx, buffer);

            for (int i = 0; i < SHA_DIGEST_LENGTH; i ++) {
                ss << std::hex << (int)buffer[i];
            }
            
            return ss.str();
        }

        /**
         * 简化调用
         * @param  data   需要加密的数据
         * @param  length 数据长度
         * @return        加密结果
         */
        std::string sha1_hex(char* data, size_t length) {
            SHA_CTX ctx;

            if (sha1_init(&ctx)) {
                if (sha1_update(&ctx, data, length)) {
                    return sha1_hex_digest(&ctx);
                }
            }

            return "";
        }

        /**
         * 简化调用
         * @param  data   需要加密的数据
         * @param  length 数据长度
         * @param  buffer 接收结果的内存
         * @return        true/false
         */
        bool sha1(char* data, size_t length, unsigned char* buffer) {
            SHA_CTX ctx;

            if (sha1_init(&ctx)) {
                if (sha1_update(&ctx, data, length)) {
                    sha1_digest(&ctx, buffer);
                    return true;
                }
            }

            return false;
        }
    }
}

#endif