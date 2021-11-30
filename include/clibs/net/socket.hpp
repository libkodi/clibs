#ifndef _CLIBS_SOCKET_H_
#define _CLIBS_SOCKET_H_ 1

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#include <unistd.h>
#include <iostream>
#include <functional>

#ifdef _WIN32
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
typedef int socklen_t;
typedef char SOCK_OPTVAL;
#else
typedef int SOCKET;
typedef void SOCK_OPTVAL;
#endif

namespace clibs {
    namespace net {
        /**
         * 保存socket基本信息的结构体
         */
        typedef struct {
            SOCKET sockfd;
            struct sockaddr_in addr;
        } socket_t;

    /**
     *  windows上加载和释放ws2_32的两个函数
     */
    #ifdef _WIN32
        /**
         * 加载ws2_32.dll
         * @param  num_main_version 主版本号
         * @param  num_sub_version  副版本号
         * @return                  是否加载成功
         */
        bool socket_winsock_init(int num_main_version, int num_sub_version) {
            static WSADATA nlib_ws_data;

            if (WSAStartup(MAKEWORD(num_main_version, num_sub_version), &nlib_ws_data) != 0) {
                return false;
            }

            return true;
        }

        /**
         * 释放ws2_32.dll
         */
        void socket_winsock_free() {
            WSACleanup();
        }
    #endif

        /**
         * 创建一个socket套字节
         * @param  family   ip协议簇
         * @param  type     套字节类型
         * @param  protocol 套字节协议
         * @param  sock     socket_t
         * @return          套字节描述符
         */
        SOCKET socket_new(int family, int type, int protocol, socket_t* sock) {
            sock->sockfd = socket(family, type, protocol);
            return sock->sockfd;
        }

        /**
         * 填充socket_t的ip与端口信息
         * @param  sock socket_t
         * @param  host 主机名称
         * @param  port 主机端口
         * @return      是否填充成功
         */
        bool socket_set_address(socket_t* sock, const char* host, unsigned int port) {
            hostent *hp;

            hp = gethostbyname(host);

            if (hp == NULL) {
                return false;
            } else {
                sock->addr.sin_family = hp->h_addrtype;
                sock->addr.sin_port = htons(port);
                char* ip_address = inet_ntoa(*((struct in_addr*)hp->h_addr_list[0]));
                sock->addr.sin_addr.s_addr = inet_addr(ip_address);
            }

            return true;
        }

        /**
         * 从sockaddr_in中提取端口
         * @param  sock socket_t
         * @return      端口号
         */
        int socket_get_port(const socket_t* sock) {
            return ntohs(sock->addr.sin_port);
        }

        /**
         * 从sockaddr_in中提取ip地址
         * @param  sock socket_t
         * @return      ip地址
         */
        std::string socket_get_ip(const socket_t* sock) {
            int len = sock->addr.sin_family == AF_INET6 ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
            char ip[len];
            struct in_addr in = sock->addr.sin_addr;
    #ifdef _WIN32
            return inet_ntoa(in);
    #else
            inet_ntop(sock->addr.sin_family, &in, ip, sizeof(ip));
            return ip;
    #endif
        }

        /**
         * 关闭套字节
         * @param sock socket_t
         */
        void socket_close(const socket_t* sock) {
            if (sock->sockfd != -1) {
                close(sock->sockfd);
            }
        }

        /**
         * 关闭套字节
         * @param sock sockete_t
         * @param how  关闭的类型
         */
        void socket_shutdown(const socket_t* sock, unsigned int how) {
            if (sock->sockfd != -1) {
                shutdown(sock->sockfd, how);
            }
        }

        /**
         * 依据填充地址信息后的socket_t进行bind操作
         * @param  sock socket_t
         * @return      是否绑定成功
         */
        bool socket_bind(const socket_t* sock) {
            socklen_t length = sizeof(sock->addr);

            return bind(sock->sockfd, (struct sockaddr*)&sock->addr, length) != -1;
        }

        /**
         * 通过指定主机与端口绑定socket
         * @param  sock socket_t
         * @param  host 主机地址
         * @param  port 端口
         * @return      是否绑定成功
         */
        bool socket_bind(socket_t* sock, const char* host, unsigned int port) {
            if (!socket_set_address(sock, host, port)) {
                return false;
            }

            return socket_bind(sock);
        }

        /**
         * 设定监听数量
         * @param  sock    socket_t
         * @param  backlog 监听数量
         * @return         是否设置成功
         */
        bool socket_listen(const socket_t* sock, unsigned int backlog) {
            return listen(sock->sockfd, backlog) != -1;
        }

        /**
         * 接收一个连接
         * @param  sock       主连接socket_t
         * @param  new_socket 用来接收新连接的socket_t
         * @return            新连接的socket描述符
         */
        SOCKET socket_accept(const socket_t* sock, socket_t* new_socket) {
            socklen_t len = sizeof(new_socket->addr);
            new_socket->sockfd = accept(sock->sockfd, (struct sockaddr*)&new_socket->addr, &len);
            return new_socket->sockfd;
        }

        /**
         * 依据填充地址信息后的socket_t进行connect操作
         * @param  sock socket_t
         * @return      是否连接成功
         */
        bool socket_connect(const socket_t* sock) {
            return connect(sock->sockfd, (struct sockaddr*)&sock->addr, sizeof(sock->addr)) != -1;
        }

        /**
         * 通过指定主机与端口连接
         * @param  sock socket_t
         * @param  host 主机地址
         * @param  port 端口
         * @return      是否连接成功
         */
        bool socket_connect(socket_t* sock, const char* host, unsigned int port) {
            if (!socket_set_address(sock, host, port)) {
                return false;
            }

            return socket_connect(sock);
        }

        /**
         * 接收数据
         * @param  sock   socket_t
         * @param  buffer 接收数据的缓存
         * @param  length 需要接收的长度
         * @param  flags  接收方式
         * @return        返回接收到的数据长度
         */
        int socket_recv(const socket_t* sock, char* buffer, unsigned int length, int flags) {
            return recv(sock->sockfd, buffer, length, flags);
        }

        int socket_recv(const socket_t* sock, char* buffer, unsigned int length) {
            return socket_recv(sock, buffer, length, 0);
        }

        /**
         * 读取固定大小数据通用函数
         * @param  length   读取的长度
         * @param  callback 实际读取数据的回调
         * @return          实际读取到的数据长度
         */
        int __socket_recv_must(int length, std::function<int(int)> callback) {
            int offset = 0, len = 0;

            while (offset < length) {
                len = callback(offset);

                if (len <= 0) {
                    return offset;
                } else {
                    offset += len;
                }
            }

            return offset;
        }

        /**
         * 接收指定长度的数据
         * @param  sock   socket_t
         * @param  buffer 接收数据的缓存
         * @param  length 需要接收的长度
         * @param  flags  接收方式
         * @return        返回接收到的数据长度
         */
        int socket_recv_must(const socket_t* sock, char* buffer, unsigned int length, int flags) {
            return __socket_recv_must(length, [sock, buffer, length, flags](int offset) -> int {
                return socket_recv(sock, buffer + offset, length - offset, flags);
            });
        }

        int socket_recv_must(const socket_t* sock, char* buffer, unsigned int length) {
            return socket_recv_must(sock, buffer, length, 0);
        }

        /** 接收单个字符通用函数 */
        int __socket_read_char(std::function<int(char*)> callback) {
            char buffer[1] = {0};
            int len = callback(buffer);

            return len == -1 ? -1 : len == 0 ? -2 : buffer[0];
        }

        /**
         * 接收单个字符
         * @param  sock socket_t
         * @return      返回接收到的字符
         */
        int socket_recv(const socket_t* sock) {
            return __socket_read_char([sock](char* buffer) -> int {
                return socket_recv(sock, buffer, 1);
            });
        }

        /** 读取单行数据通用函数 */
        int __socket_readline(char* buffer, std::function<int()> callback) {
            int offset = 0, c; 

            while (c != '\n') {
                c = callback();

                if (c == -1) {
                    break;
                } else {
                    *(buffer + offset) = c;
                    offset += 1;
                }
            }

            offset--;

            while(offset > 0) {
                char temp = *(buffer + offset);

                if (temp == '\n' || temp == '\r') {
                    offset--;
                } else {
                    break;
                }
            }

            buffer[++offset] = '\0';

            return offset;
        }

        /**
         * 读取单行数据
         * @param  sock   socket_t
         * @param  buffer 接收数据的缓存
         * @return        返回接收到的数据大小 
         */
        int socket_readline(const socket_t* sock, char* buffer) {
            return __socket_readline(buffer, [sock]() -> int {
                return socket_recv(sock);
            });
        }

        /**
         * 发送数据
         * @param  sock   socket_t
         * @param  data   发送数据的缓存
         * @param  length 需要发送的长度
         * @param  flags  发送方式
         * @return        已经发送的长度
         */
        int socket_send(const socket_t* sock, const char* data, unsigned int length, int flags) {
            return send(sock->sockfd, data, length, flags);
        }

        int socket_send(const socket_t* sock, const char* data, unsigned int length) {
            return socket_send(sock, data, length, 0);
        }

        /**
         * 
         * @param 
         * @param 
         * @param 
         * @param 
         * @param socket_t 
         * @return 
         */
        /**
         * 接收udp数据
         * @param  sock       socket_t
         * @param  buffer     接收数据的缓存
         * @param  length     接收长度
         * @param  flags      接收方式
         * @param  new_socket 数据来源
         * @return            接收到的数据长度
         */
        int socket_recvfrom(const socket_t* sock, char* buffer, unsigned int length, int flags, socket_t* new_socket) {
            socklen_t len = sizeof(new_socket->addr);
            return recvfrom(sock->sockfd, buffer, length, flags, (struct sockaddr*)&new_socket->addr, &len);
        }

        int socket_recvfrom(socket_t* sock, char* buffer, unsigned int length, socket_t* new_socket) {
            return socket_recvfrom(sock, buffer, length, 0, new_socket);
        }

        /**
         * 发送udp数据
         * @param  sock    socket_t
         * @param  data    发送的数据
         * @param  length  数据长度
         * @param  flags   发送方式
         * @param  to_sock 发送的目标socket_t
         * @return         已经发送长度
         */
        int socket_sendto(const socket_t* sock, const char* data, unsigned int length, int flags, socket_t* to_sock) {
            return sendto(sock->sockfd, data, length, flags, (struct sockaddr*)&to_sock->addr, sizeof(to_sock->addr));
        }

        int socket_sendto(const socket_t* sock, const char* data, unsigned int length, socket_t* to_sock) {
            return socket_sendto(sock, data, length, 0, to_sock);
        }

        /**
         * 设置阻塞
         * @param  sock  socket_t
         * @param  block 是否阻塞
         * @return       操作是否成功
         */
        bool socket_blocking(const socket_t* sock, bool block) {
    #ifdef _WIN32
            unsigned long ul = block ? 0 : 1;
            return ioctlsocket(sock->sockfd, FIONBIO, (unsigned long*)&ul) == 0;
    #else
            int flags = fcntl(sock->sockfd, F_GETFL, 0);

            if (flags < 0) {
                return false;
            }

            flags = block ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

            return !(fcntl(sock->sockfd, F_SETFL, flags) < 0);
    #endif
        }

        /**
         * 设置socket参数
         * @param  sock    socket_t
         * @param  level   级别
         * @param  optname 名称
         * @param  optval  参数值
         * @param  optlen  参数长度
         * @return         是否设置成功
         */
        bool socket_setsockopt(const socket_t* sock, int level, int optname, const SOCK_OPTVAL *optval, socklen_t optlen) {
            return setsockopt(sock->sockfd, level, optname, optval, optlen) == 0;
        }

        /**
         * 获取socket参数
         * @param  sock    socket_t
         * @param  level   级别
         * @param  optname 名称
         * @param  optval  参数值
         * @param  optlen  参数长度
         * @return         是否获取成功
         */
        bool socket_getsockopt(const socket_t* sock, int level, int optname, SOCK_OPTVAL *optval, socklen_t* optlen) {
            return getsockopt(sock->sockfd, level, optname, optval, optlen) == 0;
        }
    }
}

#endif