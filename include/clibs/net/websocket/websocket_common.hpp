#ifndef _CLIBS_WEBSOCKET_COMMON_H_
#define _CLIBS_WEBSOCKET_COMMON_H_ 1

#include <iostream>
#include <cstring>
#include <sstream>
#include "clibs/net/sslsocket.hpp"
#include "clibs/net/url.hpp"
#include "clibs/io/select.hpp"
#include "clibs/net/http/http_reader.hpp"
#include "clibs/net/http/http_header.hpp"
#include "clibs/error.hpp"
#include "clibs/random.hpp"
#include "clibs/encrypt/base64.hpp"
#include "clibs/encrypt/sha1.hpp"

#define WS_FRAME 0x00
#define WS_TEXT 0x01
#define WS_BINARY 0x02
#define WS_CLOSE 0x08
#define WS_PING 0x09
#define WS_PONG 0x0A

namespace clibs {
    namespace net {
        namespace websocket {
            typedef struct {
                bool fin;
                int opcode;
                bool mask;
                int payload_len;
                unsigned long int extend_payload_len;
                char mask_buffer[4];

                unsigned long int readed_size;
                bool eof;
            } ws_header_frame_t;

            typedef struct {
                socket_t socket;
                ssl_socket_t ssl_socket;
                unsigned int connect_timeout;
                unsigned int read_timeout;
                bool is_ssl;
                std::string error;
            } websocket_t;

            typedef void (*onopen_callback)(void* session);
            typedef void (*onerror_callback)(void* session, std::string error);
            typedef void (*onclose_callback)(void* session);
            typedef void (*onmessage_callback)(void* session, const char* data, ws_header_frame_t* pkg_info);

            /** 初始化websocket_t的一些参数 */
            void websocket_new(websocket_t* ws) {
                ws->socket.sockfd = -1;
                ws->ssl_socket.ssl = NULL;
                ws->ssl_socket.ctx = NULL;
                ws->connect_timeout = 15;
                ws->read_timeout = 15;
            }

            /** 连接服务器 */
            bool websocket_connect(websocket_t* ws, url_t* url, io::select_t* s_select, io::select_result_t* result) {
                if (!socket_set_address(&ws->socket, url->host.c_str(), url->port)) {
                    ws->error = "Unable to get host address";
                    return false;
                }

                if (!socket_new(ws->socket.addr.sin_family, SOCK_STREAM, 0, &ws->socket)) {
                    ws->error = "Failed to initialize socket";
                    return false;
                }

                if (!socket_blocking(&ws->socket, false)) {
                    ws->error = "Failed to set socket blocking";
                    return false;
                }

                socket_connect(&ws->socket);

                io::select_append(s_select, ws->socket.sockfd, ST_WRITE | ST_EXCEPT);

                int ret = io::select_wait(s_select, result, ws->connect_timeout);

                if (ret <= 0) {
                    ws->error = "Failed to connect to the server";
                    return false;
                }

                if (io::select_find(&result->exception, ws->socket.sockfd) != result->exception.end()) {
                    ws->error = "An error occurred while connecting to the server";
                    return false;
                }

                if (io::select_find(&result->write, ws->socket.sockfd) == result->write.end()) {
                    ws->error = "Output waiting failed";
                    return false;
                }

                io::select_remove(s_select, ws->socket.sockfd, ST_WRITE | ST_EXCEPT);

                if (!socket_blocking(&ws->socket, true)) {
                    ws->error = "Failed to restore socket blocking";
                    return false;
                }

                if (ws->is_ssl) {
                    if (!socket_ssl_new(&ws->ssl_socket)) {
                        ws->error = "Failed to initialize SSL";
                        return false;
                    }

                    if (!socket_ssl_bind(&ws->ssl_socket, &ws->socket)) {
                        ws->error = "Socket to SSL connection failed";
                        return false;
                    }

                    if (!socket_ssl_connect(&ws->ssl_socket)) {
                        ws->error = "SSL connection failed";
                        return false;
                    }
                }

                io::select_append(s_select, ws->socket.sockfd, ST_READ | ST_EXCEPT);
                
                return true;
            }

            /** 生成客户端握手用随机KEY */
            std::string websocket_random_key() {
                CRandom r;
                int key_length = 16;
                char content[key_length];
                size_t b64key_len = encrypt::base64_encode_size(key_length);
                char b64key[b64key_len];

                for (int i = 0; i < key_length; i ++) {
                    content[i] = r.next_int(65, 90);
                }

                encrypt::base64_encode(content, key_length, b64key, &b64key_len);
                b64key[b64key_len] = '\0';

                return b64key;
            }

            /** 发送socket数据 */
            bool websocket_send_data(websocket_t* ws, const char* data, unsigned int length) {
                int offset = 0;
                int len;

                while (offset < length) {
                    if (ws->is_ssl) {
                        len = socket_ssl_send(&ws->ssl_socket, data + offset, length - offset);
                    } else {
                        len = socket_send(&ws->socket, data + offset, length - offset);
                    }

                    if (len <= 0) {
                        ws->error = "Failed to send data";
                        return false;
                    } else {
                        offset += len;
                    }
                }

                return true;
            }

            /** 发送握手请求 */
            bool websocket_send_handshake(websocket_t* ws, url_t *url, std::string key, int version, std::string protocol) {
                std::stringstream stream;

                stream << "GET " << url->path << url->query << url->hash << " HTTP/1.1\r\n";
                stream << "Host: " << url->host << "\r\n";
                stream << "Content-Length: 0\r\n";
                stream << "Upgrade: websocket\r\n";
                stream << "Connection: upgrade\r\n";
                stream << "Sec-WebSocket-Key: " << key << "\r\n";
                stream << "Sec-WebSocket-Version: " << version << "\r\n";

                if (protocol != "") {
                    stream << "Sec-WebSocket-Protocol: " << protocol << "\r\n";
                }

                stream << "\r\n";

                std::string handshake_data = stream.str();

                const char* handshake_buffer = handshake_data.c_str();
                int handshake_buffer_size = handshake_data.length();
                int offset = 0;

                return websocket_send_data(ws, handshake_buffer, handshake_buffer_size);
            }

            /** 用来转换扩展长度 */
            unsigned long int websocket_convert_string(const char* data, unsigned int length) {
                unsigned long int val = 0;

                for (int i = 0; i < length; i ++) {
                    val |= (((int)*(data + i)) & 0xFF) << ((length - i - 1) * 8);
                }

                return val;
            }

            /** 解析握手响应 */
            bool websocket_parse_response(websocket_t* ws, http::CHttpHeader* headers, io::select_t* s_select, io::select_result_t* result) {
                http::http_reader_t reader;

                http::reader_init(&reader, &ws->socket, ws->is_ssl ? &ws->ssl_socket : NULL);
                http::reader_set_select(&reader, s_select, result, ws->read_timeout);
                
                if (!http::reader_parse_header(&reader, headers)) {
                    return false;
                } else {
                    if (reader.status_code != 101 && !headers->contains("Sec-WebSocket-Accept")) {
                        return false;
                    } else {
                        return true;
                    }
                }
            }

            /** 接收socket数据 */
            int websocket_recv_must(websocket_t* ws, char* buffer, unsigned int length) {
                if (ws->is_ssl) {
                    return socket_ssl_recv_must(&ws->ssl_socket, buffer, length);
                } else {
                    return socket_recv_must(&ws->socket, buffer, length);
                }
            }

            /** 解析websocket包的信息 */
            bool websocket_get_package_info(websocket_t* ws, ws_header_frame_t* pkg_info) {
                char two_char[2];
                int len;

                len = websocket_recv_must(ws, two_char, 2);

                if (len != 2) {
                    return false;
                } else {
                    pkg_info->fin = (two_char[0] & 0x80) == 0x80;
                    pkg_info->opcode = two_char[0] & 0x0F;
                    pkg_info->mask = (two_char[1] & 0x80) == 0x80;
                    pkg_info->payload_len = two_char[1] & 0x7F;

                    if (pkg_info->payload_len == 126) {
                        char extend_payload_len[2];
                        len = websocket_recv_must(ws, extend_payload_len, 2);

                        if (len != 2) {
                            return false;
                        } else {
                            pkg_info->extend_payload_len = websocket_convert_string(extend_payload_len, 2);
                        }
                    } else if (pkg_info->payload_len == 127) {
                        char extend_payload_len[8];
                        len = websocket_recv_must(ws, extend_payload_len, 8);

                        if (len != 8) {
                            return false;
                        } else {
                            pkg_info->extend_payload_len = websocket_convert_string(extend_payload_len, 8);
                        }
                    } else {
                        pkg_info->extend_payload_len = pkg_info->payload_len;
                    }

                    if (pkg_info->mask) {
                        len = websocket_recv_must(ws, pkg_info->mask_buffer, 4);

                        if (len != 4) {
                            return false;
                        }
                    }
                }

                return true;
            }

            /** 使用掩码加密数据 */
            void websocket_mask(const char* mask, char* data, unsigned int length) {
                int temp1, temp2;

                for (int i = 0; i < length; i ++) {
                    temp1 = (int)*(data + i);
                    temp2 = (int)mask[i % 4];

                    *(data + i) = ((~temp1) & temp2) | (temp1 & (~temp2));
                }
            }

            /** 使用掩码解密数据 */
            void websocket_unmask(const char* mask, char* buffer, unsigned int length) {
                for (int i = 0; i < length; i ++) {
                    *(buffer + i) = (int)*(buffer + i) ^ (int)mask[i % 4];
                }
            }

            /** 生成一个随机掩码 */
            void websocket_random_mask(char* buffer, unsigned int length) {
                CRandom r;

                for (int i = 0; i < length; i ++) {
                    *(buffer + i) = r.next_int(65, 90);
                }
            }

            /** 发送websocket数据包 */
            bool websocket_send(websocket_t* ws, bool is_fin, int opcode, bool use_mask, const char* data, unsigned int length) {
                int header_size = 14, real_size = 0;
                char pkg_data[length + 14], *pkg = pkg_data;

                if (is_fin) {
                    *pkg = 0x80;
                }

                *pkg++ |= opcode;

                if (use_mask) {
                    *pkg = 0x80;
                }

                if (length < 126) {
                    *pkg++ |= (length & 0x7F);
                    real_size = 2;
                } else if (length < 65536) {
                    *pkg++ |= 0x7E;
                    *pkg++ = (length >> 8) & 0xFF;
                    *pkg++ = (length >> 0) & 0xFF;
                    real_size = 4;
                } else if (length < 0xFFFFFFFF) {
                    *pkg++ |= 0x7F;

                    // *pkg++ = (length >> 56) & 0xFF;
                    // *pkg++ = (length >> 48) & 0xFF;
                    // *pkg++ = (length >> 40) & 0xFF;
                    // *pkg++ = (length >> 32) & 0xFF;
                    *pkg++ = 0;
                    *pkg++ = 0;
                    *pkg++ = 0;
                    *pkg++ = 0;

                    *pkg++ = (length >> 24) & 0xFF;
                    *pkg++ = (length >> 16) & 0xFF;
                    *pkg++ = (length >> 8) & 0xFF;
                    *pkg++ = (length >> 0) & 0xFF;
                    real_size = 10;
                }

                if (use_mask) {
                    char mask_buffer[4];
                    websocket_random_mask(mask_buffer, 4);
                    *pkg++ = mask_buffer[0];
                    *pkg++ = mask_buffer[1];
                    *pkg++ = mask_buffer[2];
                    *pkg++ = mask_buffer[3];
                    real_size += 4;

                    memcpy(pkg, data, length);
                    websocket_mask(mask_buffer, pkg, length);
                } else {
                    memcpy(pkg, data, length);
                }

                real_size += length;
                pkg_data[real_size] = '\0';

                return websocket_send_data(ws, pkg_data, real_size);
            }

            /** 基类封装 */
            class CWebSocket {
                public:
                    CWebSocket() {
                        websocket_new(&m_websocket);
                        m_onopen = NULL;
                        m_onclose = NULL;
                        m_onerror = NULL;
                        m_onmessage = NULL;
                    }

                    virtual void run(std::string ws_url) {};

                    /** 关闭websocket */
                    void close() {
                        if (m_is_quit) {
                            return;
                        }

                        m_is_quit = true;
                        socket_ssl_close(&m_websocket.ssl_socket);
                        socket_shutdown(&m_websocket.socket, SHUT_RDWR);
                        socket_close(&m_websocket.socket);
                    }

                    /** 设置连接超时时间 */
                    void set_connect_timeout(unsigned int timeout) {
                        m_websocket.connect_timeout = timeout;
                    }

                    /** 设置连接超时时间 */
                    void set_read_timeout(unsigned int timeout) {
                        m_websocket.read_timeout = timeout;
                    }

                    /** 获取错误信息 */
                    std::string error() {
                        return m_websocket.error == "" ? errstr() : m_websocket.error;
                    }

                    /**
                     * 绑定回调函数
                     */
                    void onopen(onopen_callback cb) {
                        m_onopen = cb;
                    }

                    void onclose(onclose_callback cb) {
                        m_onclose = cb;
                    }

                    void onerror(onerror_callback cb) {
                        m_onerror = cb;
                    }

                    void onmessage(onmessage_callback cb) {
                        m_onmessage = cb;
                    }

                    virtual void do_open_callback(void* session) {
                        if (m_onopen != NULL) {
                            m_onopen(session);
                        }
                    }

                    virtual void do_error_callback(void* session, std::string error) {
                        if (m_onerror != NULL) {
                            m_onerror(session, error);
                        }
                    }

                    virtual void do_close_callback(void* session) {
                        if (m_onclose != NULL) {
                            m_onclose(session);
                        }
                    }

                    virtual void do_message_callback(void* session, const char* data, ws_header_frame_t* pkg_info) {
                        if (m_onmessage != NULL) {
                            m_onmessage(session, data, pkg_info);
                        }
                    }

                protected:
                    websocket_t m_websocket;
                    bool m_is_quit;
                    onopen_callback m_onopen;
                    onerror_callback m_onerror;
                    onclose_callback m_onclose;
                    onmessage_callback m_onmessage;
            };
        }
    }
}

#endif