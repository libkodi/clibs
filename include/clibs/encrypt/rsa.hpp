#ifndef _CLIBS_RSA_H_
#define _CLIBS_RSA_H_ 1

#include <iostream>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <streambuf>
#include <functional>
#include "clibs/encrypt/base64.hpp"

#define RSA_PKCS1_KEY 1
#define RSA_PKCS8_KEY 2

namespace clibs {
    namespace encrypt {
        /**
         * 以bio方式加载公私钥
         * @param  content  公私钥内容
         * @param  key_type 公私钥的类型: RSA_PKCS1_KEY/RSA_PKCS8_KEY
         * @param  callback 实现实际加载公私钥的回调
         * @return          RSA*
         */
        RSA* __rsa_load_key(const char* content, int key_type, std::function<RSA*(BIO*, int)> callback) {
            RSA *rsa = NULL;
            BIO *key_bio;

            key_bio = BIO_new_mem_buf((void*)content, -1);

            if (key_bio == NULL) {
                return NULL;
            }

            rsa = callback(key_bio, key_type);
            
            BIO_free_all(key_bio);
            return rsa;
        }

        /** 依据公钥内容加载RSA */
        RSA* rsa_load_bio_public_key(const char* pubkey, int key_type) {
            return __rsa_load_key(pubkey, key_type, [](BIO* key_bio, int key_type) -> RSA* {
                EVP_PKEY* evp_key = NULL;
                RSA* rsa = NULL;

                if (key_type == RSA_PKCS8_KEY) {
                    evp_key = PEM_read_bio_PUBKEY(key_bio, NULL, NULL, NULL);
                    rsa = EVP_PKEY_get1_RSA(evp_key);
                } else {
                    rsa = PEM_read_bio_RSAPublicKey(key_bio, &rsa, NULL, NULL);
                }

                if (evp_key != NULL) {
                    EVP_PKEY_free(evp_key);
                }

                return rsa;
            });
        }

        /** 依据内容加载PKCS1公钥 */
        RSA* rsa_load_bio_pkcs1_pubkey(const char* content) {
            return rsa_load_bio_public_key(content, RSA_PKCS1_KEY);
        }

        /** 依据内容加载PKCS8公钥 */
        RSA* rsa_load_bio_pkcs8_pubkey(const char* content) {
            return rsa_load_bio_public_key(content, RSA_PKCS8_KEY);
        }

        /** 依据公钥文件加载RSA */
        RSA* rsa_load_public_key(const char* filename, int key_type) {
            std::ifstream file(filename);

            if (!file.is_open()) {
                return NULL;
            }

            std::string key_content((std::istreambuf_iterator<char>(file)),  std::istreambuf_iterator<char>());
            file.close();
            return rsa_load_bio_public_key(key_content.c_str(), key_type);
        }

        /** 依据文件加载PKCS1公钥 */
        RSA* rsa_load_pkcs1_pubkey(const char* file) {
            return rsa_load_public_key(file, RSA_PKCS1_KEY);
        }

        /** 依据文件加载PKCS8公钥 */
        RSA* rsa_load_pkcs8_pubkey(const char* file) {
            return rsa_load_public_key(file, RSA_PKCS8_KEY);
        }

        /** 依据私钥内容加载RSA */
        RSA* rsa_load_bio_private_key(const char* content, int key_type) {
            return __rsa_load_key(content, key_type, [](BIO* key_bio, int key_type) -> RSA* {
                EVP_PKEY* evp_key = NULL;
                RSA* rsa = NULL;

                if (key_type == RSA_PKCS8_KEY) {
                    evp_key = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL);
                    rsa = EVP_PKEY_get1_RSA(evp_key);
                } else {
                    rsa = PEM_read_bio_RSAPrivateKey(key_bio, &rsa, NULL, NULL);
                }

                if (evp_key != NULL) {
                    EVP_PKEY_free(evp_key);
                }

                return rsa;
            });
        }

        /** 依据内容加载PKCS1私钥 */
        RSA* rsa_load_bio_pkcs1_prikey(const char* content) {
            return rsa_load_bio_private_key(content, RSA_PKCS1_KEY);
        }

        /** 依据内容加载PKCS8私钥 */
        RSA* rsa_load_bio_pkcs8_prikey(const char* content) {
            return rsa_load_bio_private_key(content, RSA_PKCS8_KEY);
        }

        /** 依据私钥文件加载RSA */
        RSA* rsa_load_private_key(const char* filename, int key_type) {
            std::ifstream file(filename);

            if (!file.is_open()) {
                return NULL;
            }


            std::string key_content((std::istreambuf_iterator<char>(file)),  std::istreambuf_iterator<char>());
            file.close();
            return rsa_load_bio_private_key(key_content.c_str(), key_type);
        }

        /** 依据文件加载PKCS1私钥 */
        RSA* rsa_load_pkcs1_prikey(const char* file) {
            return rsa_load_private_key(file, RSA_PKCS1_KEY);
        }

        /** 依据文件加载PKCS8私钥 */
        RSA* rsa_load_pkcs8_prikey(const char* file) {
            return rsa_load_private_key(file, RSA_PKCS8_KEY);
        }

        /**
         * 加签与验签的通用处理
         * @param  rsa      RSA*
         * @param  callback 加签与验签的实现回调
         * @return          true/false
         */
        bool __rsa_sign_with_verify(RSA* rsa, std::function<bool(EVP_MD_CTX*, EVP_PKEY*)> callback) {
            EVP_MD_CTX* evp_ctx = EVP_MD_CTX_create();
            EVP_PKEY* evp_key  = EVP_PKEY_new();
            bool result = false;

            if (evp_key != NULL) {
                EVP_PKEY_set1_RSA(evp_key, rsa);
                result = callback(evp_ctx, evp_key);
            }

#ifdef _WIN32
            EVP_MD_CTX_cleanup(evp_ctx);
#else
            EVP_MD_CTX_free(evp_ctx);
#endif
            
            if (evp_key != NULL) {
                EVP_PKEY_free(evp_key);
            }

            return result;
        }

        /**
         * 使用私钥生成签名
         * @param  rsa_private_key 私钥
         * @param  message         加签文本
         * @param  message_length  文本长度
         * @param  evp_method      加签方法
         * @param  buffer          加签结果
         * @param  buffer_length   加载结果内容长度
         * @return                 true/false
         */
        bool rsa_sign(RSA* rsa, const char* message, size_t message_length, const EVP_MD* evp_method, char* outbuf, size_t* outlen) {
            return __rsa_sign_with_verify(rsa, [message, message_length, evp_method, outbuf, outlen](EVP_MD_CTX* evp_ctx, EVP_PKEY* evp_key) -> bool {
                bool result = false;
                char buffer[1024];
                size_t buffer_length;

                if (EVP_DigestSignInit(evp_ctx,NULL, evp_method, NULL, evp_key) > 0) {
                    if (EVP_DigestSignUpdate(evp_ctx, message, message_length) > 0) {
                        if (EVP_DigestSignFinal(evp_ctx, NULL, &buffer_length) > 0) {
                            if (EVP_DigestSignFinal(evp_ctx, (unsigned char*)buffer, &buffer_length) > 0) {
                                result = true;
                            }
                        }
                    }
                }

                if (result) {
                    base64_encode(buffer, buffer_length, outbuf, outlen);
                }

                return result;
            });
        }

        /**
         * 验签
         * @param  rsa         公钥
         * @param  signature   签名
         * @param  sign_length 签名长度
         * @param  data        验签数据
         * @param  data_length 验签数据长度
         * @param  evp_method  验签方法
         * @return             true/false
         */
        bool rsa_verify(RSA* rsa, const char* signature, size_t sign_length, const char* data, size_t data_length, const EVP_MD* evp_method) {
            return __rsa_sign_with_verify(rsa, [signature, data, data_length, evp_method, sign_length](EVP_MD_CTX* evp_ctx, EVP_PKEY* evp_key) -> bool {
                bool result = false;
                size_t design_length;
                char design[sign_length];
                base64_decode(signature, sign_length, design, &design_length);

                if (EVP_DigestVerifyInit(evp_ctx, NULL, evp_method, NULL, evp_key) > 0) {
                    if (EVP_DigestVerifyUpdate(evp_ctx, data, data_length) > 0) {
                        if (EVP_DigestVerifyFinal(evp_ctx, (unsigned char*)design, design_length) == 1) {
                            result = true;
                        }
                    }
                }

                return result;
            });
        }

        /**
         * 释放RSA
         */
        void rsa_free(RSA* rsa) {
            RSA_free(rsa);
        }

        /** 生成rsa密钥对 */
        RSA* rsa_keypair(unsigned int key_length) {
            return RSA_generate_key(key_length, RSA_F4, NULL, NULL);
        }

        /**
         * 读取公私钥结果的通用处理
         * @param  rsa      RSA*
         * @param  result   接收结果的字符串引用
         * @param  key_type 读取或需要转换的类型: RSA_PKCS1_KEY/RSA_PKCS8_KEY
         * @param  callback 多RSA*加读取公私钥的回调实现
         * @return          true/false
         */
        bool __rsa_read_key(RSA* rsa, std::string& result, int key_type, std::function<int(RSA*, EVP_PKEY*, BIO*, int)> callback) {
            BIO *key_bio = BIO_new(BIO_s_mem());
            size_t key_length;
            int len;
            bool ret = false;
            EVP_PKEY* evp_key;

            if (key_bio == NULL) {
                return false;
            }

            if (key_type == RSA_PKCS8_KEY) {
                evp_key = EVP_PKEY_new();

                if (evp_key != NULL) {
                    EVP_PKEY_set1_RSA(evp_key, rsa);
                }
            }

            len = callback(rsa, evp_key, key_bio, key_type);

            if (len > 0) {
                key_length = BIO_pending(key_bio);
                char buffer[key_length];
                BIO_read(key_bio, buffer, key_length);
                buffer[key_length] = '\0';
                result = buffer;
                ret = true;
            }

            BIO_free_all(key_bio);

            if (key_type == RSA_PKCS8_KEY) {
                if (evp_key != NULL) {
                    EVP_PKEY_free(evp_key);
                }
            }

            return ret;
        }

        /** 获取公钥 */
        bool rsa_read_public_key(RSA* rsa, std::string& public_key, int key_type) {
            return __rsa_read_key(rsa, public_key, key_type, [](RSA* rsa, EVP_PKEY* evp_key, BIO* key_bio, int key_type) -> int {
                return key_type != RSA_PKCS8_KEY ? PEM_write_bio_RSAPublicKey(key_bio, rsa) : PEM_write_bio_PUBKEY(key_bio, evp_key);
            });
        }

        /** 获取PKCS1公钥 */
        bool rsa_read_pkcs1_pubkey(RSA* rsa, std::string& public_key) {
            return rsa_read_public_key(rsa, public_key, RSA_PKCS1_KEY);  
        }

        /** 获取PKCS8公钥 */
        bool rsa_read_pkcs8_pubkey(RSA* rsa, std::string& public_key) {
            return rsa_read_public_key(rsa, public_key, RSA_PKCS8_KEY);  
        }

        /** 获取私钥 */
        bool rsa_read_private_key(RSA* rsa, std::string& private_key, int key_type) {
            return __rsa_read_key(rsa, private_key, key_type, [](RSA* rsa, EVP_PKEY* evp_key, BIO* key_bio, int key_type) -> int {
                return key_type != RSA_PKCS8_KEY ? PEM_write_bio_RSAPrivateKey(key_bio, rsa, NULL, NULL, 0, NULL, NULL) : PEM_write_bio_PKCS8PrivateKey(key_bio, evp_key, NULL, NULL, 0, NULL, NULL);
            });
        }

        /** 获取PKCS1私钥 */
        bool rsa_read_pkcs1_prikey(RSA* rsa, std::string& private_key) {
            return rsa_read_private_key(rsa, private_key, RSA_PKCS1_KEY);  
        }

        /** 获取PKCS8私钥 */
        bool rsa_read_pkcs8_prikey(RSA* rsa, std::string& private_key) {
            return rsa_read_private_key(rsa, private_key, RSA_PKCS8_KEY);  
        }

        /** 计算加密后的数据大小 */
        size_t rsa_encrypt_size(RSA* rsa, size_t data_length) {
            int rsa_len = RSA_size(rsa);
            int enctype_size = rsa_len - 11;

            return std::ceil((double)data_length / (double)enctype_size) * rsa_len;
        }

        /** 计算加密后返回的base64格式的内容大小 */
        size_t rsa_encrypt_base64_size(RSA* rsa, size_t data_length) {
            return base64_encode_size(rsa_encrypt_size(rsa, data_length));
        }

        /**
         * 加密通用处理
         * @param  rsa         公私钥
         * @param  data        需要加密的数据
         * @param  data_length 加密数据长度
         * @param  outbuf      结果输出缓存
         * @param  outlen      结果输出长度
         * @param  callback    加密回调
         * @return             true/false
         */
        bool __rsa__encrypt(RSA* rsa, const char* data, size_t data_length, char* outbuf, size_t* outlen, std::function<int(char*, size_t, int, int, size_t, char*)> callback) {
            int rsa_len = RSA_size(rsa), pkg_size = rsa_len - 11;
            size_t offset = 0;
            size_t total = rsa_encrypt_size(rsa, data_length);
            int len;
            char buffer[total];
            char encrypt_package[pkg_size];
            size_t encrypt_size, pkg_index = 0;

            *outlen = 0;

            while(offset < data_length) { // 分割传入的内容进行加密，实现支持超长文本加密
                memset(encrypt_package, 0, pkg_size);
                encrypt_size = data_length - offset;
                encrypt_size = encrypt_size > pkg_size ? pkg_size : encrypt_size;
                memcpy(encrypt_package, data + offset, encrypt_size);
                encrypt_package[encrypt_size] = '\0';

                len = callback(encrypt_package, encrypt_size, rsa_len, pkg_size, pkg_index, buffer);

                if (len < 0) {
                    return false;
                }

                offset += encrypt_size;
                pkg_index ++;
            }

            base64_encode(buffer, total, outbuf, outlen);

            return true;
        }

        /** 公钥加密 */
        bool rsa_public_encrypt(RSA* rsa, const char* data, size_t data_length, int padding, char* outbuf, size_t* outlen) {
            return __rsa__encrypt(rsa, data, data_length, outbuf, outlen, [rsa, padding](char* encrypt_data, size_t encrypt_size, int rsa_len, int pkg_size, size_t pkg_index, char* buffer) -> int {
                return RSA_public_encrypt(encrypt_size, (unsigned char*)encrypt_data, (unsigned char*)(buffer + (pkg_index * rsa_len)), rsa, padding);;
            });
        }

        /** 私钥加密 */
        bool rsa_private_encrypt(RSA* rsa, const char* data, size_t data_length, int padding, char* outbuf, size_t* outlen) {
            return __rsa__encrypt(rsa, data, data_length, outbuf, outlen, [rsa, padding](char* encrypt_data, size_t encrypt_size, int rsa_len, int pkg_size, size_t pkg_index, char* buffer) -> int {
                return RSA_private_encrypt(encrypt_size, (unsigned char*)encrypt_data, (unsigned char*)(buffer + (pkg_index * rsa_len)), rsa, padding);;
            });
        }

        /** 计算解密后数据的大小 */
        size_t rsa_decrypt_size(size_t encrypt_size) {
            return base64_decode_size(encrypt_size);
        }

        /**
         * 解密的通用处理
         * @param  rsa         公私钥
         * @param  data        密文数据
         * @param  data_length 密文数据长度
         * @param  outlen      解密后的结果长度
         * @param  callback    实际解密数据回调
         * @return             true/false
         */
        bool __rsa_decrypt(RSA* rsa, const char* data, size_t data_length, size_t* outlen, std::function<int(char*,int,size_t)> callback) {
            char encrypt_data[base64_decode_size(data_length)];
            size_t encrypt_length;
            size_t offset = 0, result_offset = 0;
            int len, rsa_len = RSA_size(rsa);
            char decrypt_data[rsa_len];

            base64_decode(data, data_length, encrypt_data, &encrypt_length);
            
            *outlen = 0;

            while(offset < encrypt_length) { // 分割解密，支持超长密文解密
                memset(decrypt_data, 0, rsa_len);
                memcpy(decrypt_data, encrypt_data, rsa_len);

                len = callback(decrypt_data, rsa_len, result_offset);

                if (len < 0) {
                    return false;
                }

                result_offset += len;
                offset += rsa_len;
            }

            *outlen = result_offset;

            return true;
        }

        /** 公钥解密 */
        bool rsa_public_decrypt(RSA* rsa, const char* data, size_t data_length, int padding, char* outbuf, size_t* outlen) {
            return __rsa_decrypt(rsa, data, data_length, outlen, [rsa, padding, outbuf, outlen](char* decrypt_data, int pkg_size, size_t result_offset) -> int {
                return RSA_public_decrypt(pkg_size, (unsigned char*)decrypt_data, (unsigned char*)(outbuf + result_offset), rsa, padding);;
            }); 
        }

        /** 私钥解密 */
        bool rsa_private_decrypt(RSA* rsa, const char* data, size_t data_length, int padding, char* outbuf, size_t* outlen) {
            return __rsa_decrypt(rsa, data, data_length, outlen, [rsa, padding, outbuf, outlen](char* decrypt_data, int pkg_size, size_t result_offset) -> int {
                return RSA_private_decrypt(pkg_size, (unsigned char*)decrypt_data, (unsigned char*)(outbuf + result_offset), rsa, padding);;
            }); 
        }
    }
}

#endif