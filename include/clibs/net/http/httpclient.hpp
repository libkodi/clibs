#ifndef _CLIBS_HTTP_CLIENT_H_
#define _CLIBS_HTTP_CLIENT_H_ 1

#include <iostream>
#include <map>
#include <sstream>
#include "clibs/net/sslsocket.hpp"
#include "clibs/net/url.hpp"
#include "clibs/io/select.hpp"
#include "clibs/error.hpp"
#include "clibs/net/http/http_header.hpp"
#include "clibs/net/http/http_reader.hpp"

namespace clibs {
    namespace net {
        namespace http {
            /**
             * http客户端封装
             */
            class CHttpClient {
                protected:
                    url_t m_url; // url信息
                    socket_t m_socket; // socket信息
                    ssl_socket_t m_ssl_socket; // ssl_socket信息
                    io::select_t m_select; // select信息
                    io::select_result_t m_select_result; // select结果保存目标
                    http_reader_t m_reader; // http读取器
                    CHttpHeader m_fields; // 响应头信息
                    bool m_is_ssl; // 是否启用ssl连接
                    std::string m_method; // 请求方法
                    CHttpHeader m_headers; // 请求头信息
                    unsigned int m_connect_timeout; // 连接超时时间
                    unsigned int m_read_timeout; // 读取超时时间
                    std::string m_error;
                    bool m_closed;
                public:
                    /**
                     * 初始化部分变量
                     */
                    CHttpClient() {
                        m_socket.sockfd = -1;
                        m_ssl_socket.ssl = NULL;
                        m_ssl_socket.ctx = NULL;
                        m_connect_timeout = 15;
                        m_read_timeout = 15;
                        m_closed = false;
                        io::select_init(&m_select);
                    }

                    /**
                     * 设置请求url
                     * @param  url 请求url
                     * @return     true/false
                     */
                    bool set_url(std::string url) {
                        if (!url_parse(&m_url, url)) {
                            m_error = "Failed to parse URL.";
                        }

                        if (m_url.protocol == "https") {
                            m_is_ssl = true;
                        } else {
                            m_is_ssl = false;
                        }

                        return true;
                    }

                    /**
                     * 设置请求方法
                     * @param method 请求方法
                     */
                    void set_method(std::string method) {
                        m_method = method;
                    }

                    /**
                     * 添加请求头
                     * @param name  参数名
                     * @param value 参数值
                     */
                    void add_header(std::string name, std::string value) {
                        m_headers.append(name, value);
                    }

                    /** 添加多个请求头 */
                    void add_headers(CHttpHeader* headers) {
                        m_headers = CHttpHeader(headers->data());
                    }

                    /**
                     * 设置连接超时
                     * @param timeout 超时时间
                     */
                    void set_connect_timeout(unsigned int timeout) {
                        m_connect_timeout = timeout;
                    }

                    /**
                     * 设置读取超时时间
                     * @param timeout 超时时间
                     */
                    void set_read_timeout(unsigned int timeout) {
                        m_read_timeout = timeout;
                    }

                    /**
                     * 关闭httpclient
                     */
                    void close() {
                        if (m_closed) {
                            return;
                        }

                        if (m_ssl_socket.ctx != NULL) {
                            socket_ssl_close(&m_ssl_socket);
                        }

                        if (m_socket.sockfd != -1) {
                            socket_shutdown(&m_socket, SHUT_RDWR);
                            socket_close(&m_socket);
                        }

                        m_closed = true;
                    }

                    /**
                     * 发送请求头
                     */
                    bool __send_header() {
                        std::stringstream data;

                        data << m_method << " " << m_url.path << m_url.query << m_url.hash << " HTTP/1.1" << "\r\n";
                        data << "Host: " << m_url.host << "\r\n";
                        data << m_headers.str();
                        data << "\r\n";

                        std::string header_string = data.str();
                        const char* header = header_string.c_str();

                        return __send_data(header, strlen(header));
                    }

                    /**
                     * 循环发送完整数据
                     * @param    data       需要发送的数据
                     * @param    length     数据长度
                     */
                    bool __send_data(const char* data, int length) {
                        int offset = 0, len;

                        while (offset < length) {
                            if (m_is_ssl) {
                                len = socket_ssl_send(&m_ssl_socket, data + offset, length - offset);
                            } else {
                                len = socket_send(&m_socket, data + offset, length - offset);
                            }

                            if (len <= 0) {
                                m_error = "Failed to send request data";
                                return false;
                            }

                            offset += len;
                        }

                        return true;
                    }

                    /**
                     * 连接服务器并发送请求头
                     */
                    bool connect() {
                        if (!socket_set_address(&m_socket, m_url.host.c_str(), m_url.port)) {
                            m_error = "Unable to get host address";
                            return false;
                        }

                        if (!socket_new(m_socket.addr.sin_family, SOCK_STREAM, 0, &m_socket)) {
                            m_error = "Failed to initialize socket";
                            return false;
                        }

                        if (!socket_blocking(&m_socket, false)) {
                            m_error = "Failed to set socket blocking";
                            return false;
                        }

                        socket_connect(&m_socket);

                        io::select_append(&m_select, m_socket.sockfd, ST_WRITE | ST_EXCEPT);

                        int ret = io::select_wait(&m_select, &m_select_result, m_connect_timeout);

                        if (ret <= 0) {
                            m_error = "Failed to connect to the server";
                            return false;
                        }

                        if (io::select_find(&m_select_result.exception, m_socket.sockfd) != m_select_result.exception.end()) {
                            m_error = "An error occurred while connecting to the server";
                            return false;
                        }

                        if (io::select_find(&m_select_result.write, m_socket.sockfd) == m_select_result.write.end()) {
                            m_error = "Output waiting failed";
                            return false;
                        }

                        io::select_remove(&m_select, m_socket.sockfd, ST_WRITE | ST_EXCEPT);

                        if (!socket_blocking(&m_socket, true)) {
                            m_error = "Failed to restore socket blocking";
                            return false;
                        }

                        if (m_is_ssl) {
                            if (!socket_ssl_new(&m_ssl_socket)) {
                                m_error = "Failed to initialize SSL";
                                return false;
                            }

                            if (!socket_ssl_bind(&m_ssl_socket, &m_socket)) {
                                m_error = "Socket to SSL connection failed";
                                return false;
                            }

                            if (!socket_ssl_connect(&m_ssl_socket)) {
                                m_error = "SSL connection failed";
                                return false;
                            }
                        }
                        
                        return __send_header();
                    }

                    /**
                     * 发送继头部信息外的主体数据
                     * @param    data       需要发送的数据
                     * @param    length     数据长度                     */
                    bool send(const char* data, int length) {
                        return __send_data(data, length);
                    }

                    /**
                     * 获取响应
                     * @return   true/false
                     */
                    bool get_response() {
                        io::select_append(&m_select, m_socket.sockfd, ST_READ | ST_EXCEPT);
                        reader_init(&m_reader, &m_socket, m_is_ssl ? &m_ssl_socket : NULL);
                        reader_set_select(&m_reader, &m_select, &m_select_result, m_read_timeout);
                        return reader_parse_header(&m_reader, &m_fields);
                    }

                    /** 获取响应码 */
                    int get_status_code() {
                        return m_reader.status_code;
                    }

                    /** 获取响应状态描述 */
                    std::string get_status_message() {
                        return m_reader.error;
                    }

                    /** 读取全部响应body */
                    std::string read() {
                        return reader_full_read(&m_reader);
                    }

                    /** 获取errno */
                    int get_errno() {
                        return m_select_result.errnos[0];
                    }

                    /** 获取所有指定响应头信息 */
                    std::vector<std::string> get_fields(std::string name) {
                        return m_headers.get_fields(name);
                    }

                    /** 从响应头信息中依据下标获取内容 */
                    std::string get_field(std::string name, int index) {
                        return m_headers.get(name, index);
                    }

                    /** 获取响应头信息 */
                    std::string get_field(std::string name) {
                        return m_headers.get(name);
                    }

                    /** 获取整个响应头 */
                    CHttpHeader* get_response_headers() {
                        return &m_headers;
                    }

                    /** 获取错误描述 */
                    std::string error() {
                        return m_error == "" ? errstr() : m_error;
                    }
            };
        }
    }
}

#endif