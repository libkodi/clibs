#ifndef _CLIBS_SELECT_H_
#define _CLIBS_SELECT_H_ 1

#include <algorithm>
#include <vector>
#include "clibs/net/socket.hpp"

#define ST_READ 0x02
#define ST_WRITE 0x04
#define ST_EXCEPT 0x08

namespace clibs {
    namespace io {
        /**
         * 一个保存sockfd的数组
         */
        typedef std::vector<SOCKET> vec_sockfd_list;

        /**
         * 用来获取结果的结构体，select成功的sockfd将写入对应的数组中
         */
        typedef struct {
            vec_sockfd_list read;
            vec_sockfd_list write;
            vec_sockfd_list exception;
            std::vector<int> errnos;
            struct timeval timeout;
        } select_result_t;

        /**
         * select的主结构体
         */
        typedef struct {
            fd_set read;
            fd_set write;
            fd_set exception;
            SOCKET max_sockfd; // 记录当前监听的最大的sockfd
            vec_sockfd_list sockfd_array; // 保存所有添加的sockfd
        } select_t;
            
        /**
         * 初始化select_t
         * @param s select_t
         */
        void select_init(select_t* s) {
            FD_ZERO(&s->read);
            FD_ZERO(&s->write);
            FD_ZERO(&s->exception);
            s->max_sockfd = -1;
        }

        /**
         * 从一个数据中查找指定sockfd
         * @param  s      sockfd数组
         * @param  sockfd 需要查找的sockfd
         * @return        结果指针
         */
        vec_sockfd_list::iterator select_find(vec_sockfd_list* s, SOCKET sockfd) {
            for (vec_sockfd_list::iterator it = s->begin(); it != s->end(); it ++) {
                if (*it == sockfd) {
                    return it;
                }
            }

            return s->end();
        }

        /**
         * 添加一个sockfd到select监听
         * @param s      select_t
         * @param sockfd 
         * @param mode   监听的模式
         */
        void select_append(select_t* s, SOCKET sockfd, int mode) {
            vec_sockfd_list::iterator iter = select_find(&s->sockfd_array, sockfd);

            if (iter == s->sockfd_array.end()) { // 如果不在主列表中
                if (mode & ST_READ) {
                    FD_SET(sockfd, &s->read);
                }

                if (mode & ST_WRITE) {
                    FD_SET(sockfd, &s->write);
                }

                if (mode & ST_EXCEPT) {
                    FD_SET(sockfd, &s->exception);
                }

                s->sockfd_array.push_back(sockfd);
                std::sort(s->sockfd_array.begin(), s->sockfd_array.end()); // 重新排序列表
            }

            if (sockfd > s->max_sockfd) {
                s->max_sockfd = sockfd;
            }
        }

        /**
         * 判断对应的模式中是否包含该sockfd的监听
         * @param  s      socket_t
         * @param  sockfd 
         * @param  mode   监听的模式
         * @return        true/false
         */
        bool select_contains(select_t* s, SOCKET sockfd, int mode) {
            if (mode & ST_READ) {
                return FD_ISSET(sockfd, &s->read);
            }

            if (mode & ST_WRITE) {
                return FD_ISSET(sockfd, &s->write);
            }

            if (mode & ST_EXCEPT) {
                return FD_ISSET(sockfd, &s->exception);
            }

            return false;
        }

        /**
         * 重置最大sockfd
         * @param s select_t
         */
        void __reset_max_sockfd(select_t* s) {
            if (s->sockfd_array.size() > 0) {
                vec_sockfd_list::iterator iter = s->sockfd_array.end();
                iter --;
                s->max_sockfd = *iter;
            } else {
                s->max_sockfd = -1;
            }
        }

        /**
         * 移除sockfd的监听
         * @param s      select_t
         * @param sockfd 
         * @param mode   监听的模式
         */
        void select_remove(select_t* s, SOCKET sockfd, int mode) {
            if (mode & ST_READ) {
                FD_CLR(sockfd, &s->read);
            }

            if (mode & ST_WRITE) {
                FD_CLR(sockfd, &s->write);
            }

            if (mode & ST_EXCEPT) {
                FD_CLR(sockfd, &s->exception);
            }

            if (!select_contains(s, sockfd, ST_READ) && !select_contains(s, sockfd, ST_WRITE) && !select_contains(s, sockfd, ST_EXCEPT)) {
                vec_sockfd_list::iterator iter = select_find(&s->sockfd_array, sockfd);

                if (iter != s->sockfd_array.end()) {
                    s->sockfd_array.erase(iter);
                }

                if (sockfd == s->max_sockfd) {
                    __reset_max_sockfd(s);
                }
            }
        }
        
        /**
         * 获取select结果
         * @param  s           select_t
         * @param  result      select_result_t
         * @param  timeout_sec 超时时间
         * @return             select执行结果
         */
        int select_wait(select_t* s, select_result_t* result, unsigned int timeout_sec) {
            result->read.clear();
            result->write.clear();
            result->exception.clear();
            result->errnos.clear();
            fd_set read_fs = s->read, write_fs = s->write, except_fs = s->exception;
            result->timeout.tv_sec = timeout_sec;
            result->timeout.tv_usec = 0;

            int ret = select(s->max_sockfd + 1, &read_fs, &write_fs, &except_fs, &result->timeout);

            if (ret > 0) {
                SOCKET fd;
                net::socket_t sock;
                int error;

                for (vec_sockfd_list::iterator it = s->sockfd_array.begin(); it != s->sockfd_array.end(); it ++) {
                    fd = *it;
                    sock.sockfd = fd;
                    error = 0;
                    socklen_t len = sizeof(error);

                    if (net::socket_getsockopt(&sock, SOL_SOCKET, SO_ERROR, (SOCK_OPTVAL*)&error, &len)) {
                        result->errnos.push_back(error);

                        if (error != 0) {
                            result->exception.push_back(fd);
                        } else {
                            if (FD_ISSET(fd, &read_fs)) {
                                result->read.push_back(fd);
                            }

                            if (FD_ISSET(fd, &write_fs)) {
                                result->write.push_back(fd);
                            }

                            if (FD_ISSET(fd, &except_fs)) {
                                result->exception.push_back(fd);
                            }
                        }
                    } else {
                        result->exception.push_back(fd);
                        result->errnos.push_back(errno);
                    }
                }
            }

            return ret;
        }
    }
}

#endif