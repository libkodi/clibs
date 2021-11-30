#ifndef _CLIBS_EPOLL_H_
#define _CLIBS_EPOLL_H_ 1

#include <iostream>
#include <vector>
#include <sys/epoll.h>
#include "clibs/net/socket.hpp"
#include "clibs/error.hpp"

#define EPOLL_READ EPOLLIN
#define EPOLL_WRITE EPOLLOUT
#define EPOLL_EXCEPT EPOLLERR

namespace clibs {
    namespace io {
        typedef struct {
            int epoll_fd;
            struct epoll_event event;
            int maxevents;
        } epoll_t;

        typedef struct {
            std::vector<SOCKET> read;
            std::vector<SOCKET> write;
            std::vector<SOCKET> exception;
            std::vector<SOCKET> errnos;
        } epoll_result_t;

        /**
         * 初始化
         * @param  epoll       
         * @param  listen_size 最大监听数量
         * @return             true/false
         */
        bool epoll_init(epoll_t* epoll, int listen_size) {
            epoll->epoll_fd = epoll_create(listen_size);
            epoll->maxevents = 128;

            return epoll->epoll_fd < 0 ? false : true;
        }

        /**
         * 设置最大获取事件数量
         * @param epoll     
         * @param maxevents wait时最大获取事件数量
         */
        void epoll_maxevents(epoll_t* epoll, int maxevents) {
            epoll->maxevents = maxevents;
        } 

        /**
         * epoll操作
         * @param  epoll  
         * @param  sockfd 套字节描述符
         * @param  option 操作类型
         * @param  events 事件值
         * @return        true/false
         */
        bool __epool_ctl(epoll_t* epoll, SOCKET sockfd, int option, int events) {
            epoll->event.data.fd = sockfd;
            epoll->event.events = events;
            return epoll_ctl(epoll->epoll_fd, option, sockfd, &epoll->event) < 0 ? false : true;
        }

        bool epoll_append(epoll_t* epoll, SOCKET sockfd, int events) {
            return __epool_ctl(epoll, sockfd, EPOLL_CTL_ADD, events);
        }

        bool epoll_remove(epoll_t* epoll, SOCKET sockfd, int events) {
            return __epool_ctl(epoll, sockfd, EPOLL_CTL_DEL, events);
        }

        /**
         * 等待查询结果
         * @param  epoll   
         * @param  result  接收结果
         * @param  timeout 超时时间
         * @return         查询到的数量
         */
        int epoll_wait(epoll_t* epoll, epoll_result_t* result, unsigned int timeout) {
            result->read.clear();
            result->write.clear();
            result->exception.clear();
            result->errnos.clear();

            struct epoll_event events[epoll->maxevents];

            int nfds = epoll_wait(epoll->epoll_fd, events, epoll->maxevents, timeout);
            int error;
            socklen_t len;
            net::socket_t sock;

            for (int i = 0; i < nfds; i ++) {
                error = 0;
                len = sizeof(error);
                sock.sockfd = events[i].data.fd;

                if (!net::socket_getsockopt(&sock, SOL_SOCKET, SO_ERROR, (SOCK_OPTVAL*)&error, &len) || error != 0) {
                    result->errnos.push_back(errno);
                    result->exception.push_back(sock.sockfd);
                    continue;
                }

                result->errnos.push_back(error);

                if (events[i].events & EPOLLIN) {
                    result->read.push_back(events[i].data.fd);
                } else if (events[i].events & EPOLLOUT) {
                    result->write.push_back(events[i].data.fd);
                } else {
                    result->exception.push_back(events[i].data.fd);
                }
            }

            return nfds;
        }

        /**
         * 从结果数组中查找指定的sockfd
         * @param  list   
         * @param  sockfd 
         * @return        true/false
         */
        std::vector<SOCKET>::iterator epoll_find(std::vector<SOCKET>* list, SOCKET sockfd) {
            for (std::vector<SOCKET>::iterator it = list->begin(); it != list->end(); it ++) {
                if (*it == sockfd) {
                    return it;
                }
            }

            return list->end();
        }

        /** 关闭epoll */
        void epoll_close(epoll_t* epoll) {
            if (epoll->epoll_fd > -1) {
                close(epoll->epoll_fd);
            }
        }
    }
}

#endif