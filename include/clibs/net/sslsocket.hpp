#ifndef _CLIBS_SSL_SOCKET_H_
#define _CLIBS_SSL_SOCKET_H_ 1

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <functional>
#include "clibs/net/socket.hpp"

#ifdef _WIN32
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

namespace clibs {
    namespace net {
        typedef struct {
            SSL *ssl;
            SSL_CTX *ctx;
        } ssl_socket_t;

        /**
         * 加载ssl
         */
        bool socket_ssl_init() {
            OpenSSL_add_all_algorithms(); 
            SSL_load_error_strings();

            if (SSL_library_init() < 0) {
                return false;
            }

            return true;
        }

        /**
         * 释放ssl
         */
        void socket_ssl_free() {
            CRYPTO_cleanup_all_ex_data();
            ERR_remove_state(0);
            CONF_modules_unload(1);
            CONF_modules_free();
            ERR_free_strings();
            ERR_remove_thread_state(NULL);
            EVP_cleanup();
        }

        /**
         * 创建一个ssl的上下文
         * @param  ssock ssl_socket_t
         * @return       是否成功
         */
        bool socket_ssl_new(ssl_socket_t* ssock, bool is_server) {
            ssock->ctx = SSL_CTX_new(is_server ? SSLv23_server_method() : SSLv23_client_method());

            return ssock->ctx != NULL;
        }

        bool socket_ssl_new(ssl_socket_t* ssock) {
            return socket_ssl_new(ssock, false);
        }

        /**
         * 为ssl_ctx添加证书
         * @param  ssock            ssl_socket_t
         * @param  cert_file        证书路径
         * @param  private_key_file 私钥路径
         * @param  cert_type        证书类型
         * @return                  是否添加成功
         */
        bool socket_ssl_add_cert(ssl_socket_t* ssock, const char* cert_file, const char* private_key_file, int cert_type, std::function<int(SSL_CTX*,const char*, const char*)> callback) {
            int ret;

            if (callback != NULL) {
                ret = callback(ssock->ctx, cert_file, private_key_file);
            } else {
                ret = SSL_CTX_use_certificate_file(ssock->ctx, cert_file, cert_type);
            }

            if (ret <= 0) {
                return false;
            }

            if (SSL_CTX_use_PrivateKey_file(ssock->ctx, private_key_file, cert_type) <= 0) {
                return false;
            }

            if (!SSL_CTX_check_private_key(ssock->ctx)) {
                return false;
            }

            return true;
        }

        bool socket_ssl_add_cert(ssl_socket_t* ssock, const char* cert_file, const char* private_key_file, int cert_type) {
            return socket_ssl_add_cert(ssock, cert_file, private_key_file, cert_type, NULL);
        }

        /**
         * 为ssl绑定socket描述符
         * @param ssock ssl_socket_t
         * @param sock  socket_t
         */
        bool socket_ssl_bind(ssl_socket_t* ssock, SSL_CTX* ctx, socket_t* sock) {
            ssock->ctx = ctx;
            ssock->ssl = SSL_new(ctx);

            if (ssock->ssl == NULL) {
                return false;
            }

            if (SSL_set_fd(ssock->ssl, sock->sockfd) == 0) {
                return false;
            }

            return true;
        }

        bool socket_ssl_bind(ssl_socket_t* ssock, socket_t* sock) {
            return socket_ssl_bind(ssock, ssock->ctx, sock);
        }

        /**
         * socket连接转ssl连接
         * @param  ssock sslsocket_
         * @return       是否成功
         */
        bool socket_ssl_connect(const ssl_socket_t* ssock) {
            return SSL_connect(ssock->ssl) != -1;
        }

        /**
         * 关闭ssl连接
         * @param ssock ssl_socket_t
         */
        void socket_ssl_close(const ssl_socket_t* ssock) {
            if (ssock->ctx != NULL) {
                SSL_CTX_free(ssock->ctx);
            }

            if (ssock->ssl != NULL) {
                SSL_free(ssock->ssl);
            }
        }

        /**
         * 接收ssl连接
         * @param  ssock ssl_socket_t
         * @return       是否成功
         */
        bool socket_ssl_accept(const ssl_socket_t* ssock) {
            return SSL_accept(ssock->ssl) != -1;
        }

        /**
         * 获取缓冲区还有多少数据可以读取
         * @param  ssock ssl_socket_t
         * @return       可读数据大小
         */
        int socket_ssl_pending(const ssl_socket_t* ssock) {
            return SSL_pending(ssock->ssl);
        }

        /**
         * 读取数据
         * @param  ssock  ssl_socket_t
         * @param  buffer 接收数据的缓存
         * @param  length 读取长度
         * @return        读取到的数据长度
         */
        int socket_ssl_recv(const ssl_socket_t* ssock, char* buffer, unsigned int length) {
            return SSL_read(ssock->ssl, buffer, length);
        }

        /**
         * 读取指定长度数据
         * @param  ssock  ssl_socket_t
         * @param  buffer 接收数据的缓存
         * @param  length 读取长度
         * @return        读取到的数据长度
         */
        int socket_ssl_recv_must(const ssl_socket_t* ssock, char* buffer, unsigned int length) {
            return __socket_recv_must(length, [ssock, buffer, length](int offset) -> int {
                return socket_ssl_recv(ssock, buffer + offset, length - offset);;
            });
        }

        /**
         * 读取单个字符
         * @param  ssock ssl_socket_t
         * @return       成功则返回获取到的字符，读取长度为0时为-2，读取长度为-1时为-1 
         */
        int socket_ssl_recv(const ssl_socket_t* ssock) {
            return __socket_read_char([ssock](char* buffer) -> int {
                return socket_ssl_recv(ssock, buffer, 1);
            });
        }

        /**
         * 读取单行数据
         * @param  ssock   ssl_socket_t
         * @param  buffer 接收数据的缓存
         * @return        返回接收到的数据大小 
         */
        int socket_ssl_readline(const ssl_socket_t* ssock, char* buffer) {
            return __socket_readline(buffer, [ssock]() -> int {
                return socket_ssl_recv(ssock);
            });
        }
        
        /**
         * 发送数据
         * @param  ssock  ssl_socket_t
         * @param  data   发送的数据
         * @param  length 数据长度
         * @return        已经发送长度
         */
        int socket_ssl_send(const ssl_socket_t* ssock, const char* data, unsigned int length) {
            return SSL_write(ssock->ssl, data, length);
        }
    }
}

#endif