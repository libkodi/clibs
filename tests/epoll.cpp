#include <iostream>
#include "clibs/net/socket.hpp"
#include "clibs/io/epoll.hpp"
#include "clibs/array.hpp"

using namespace clibs;
using namespace clibs::net;
using namespace clibs::io;

int main(int argc, char const *argv[])
{

    socket_t sock;
    epoll_t ep;

    if (!epoll_init(&ep, 1024)) {
        std::cout << "Failed to init epoll." << std::endl;
        exit(1);
    }

    if (!socket_new(AF_INET, SOCK_STREAM, 0, &sock)) {
        std::cout << "Failed to create socket." << std::endl;
        exit(1);
    }
    int val = 1;

    if (socket_setsockopt(&sock, SOL_SOCKET, SO_REUSEADDR, (void*)&val, sizeof(val))) {
        if (socket_bind(&sock, "127.0.0.1", 1234)) {
            if (socket_listen(&sock, 5)) {
                epoll_result_t result;
                int nfds;
                socket_t nsock, client;

                epoll_append(&ep, sock.sockfd, EPOLL_READ | EPOLL_EXCEPT);

                while (true) {
                    nfds = epoll_wait(&ep, &result, -1);

                    if (nfds <= 0) {
                        break;
                    } else {
                        array_foreach(&result.read, [&ep, sock, &nsock, &client](size_t index, std::vector<SOCKET>::iterator it) -> bool {
                            SOCKET sockfd = *it;

                            if (sock.sockfd == sockfd) {
                                socket_accept(&sock, &client);
                                epoll_append(&ep, client.sockfd, EPOLL_READ | EPOLL_EXCEPT);
                                std::cout << "connect: " << socket_get_ip(&client) << ":" << socket_get_port(&client) << std::endl;
                            } else {
                                nsock.sockfd = sockfd;

                                char buffer[1024];
                                int len = socket_recv(&nsock, buffer, 1024);

                                buffer[len] = '\0';

                                printf("buffer: %s\n", buffer);
                                socket_send(&nsock, "HTTP/1.1 200 OK\r\n\r\nabc", 22);
                                socket_shutdown(&nsock, SHUT_RDWR);
                                socket_close(&nsock);
                            }

                            return true;
                        });

                        array_foreach(&result.exception, [&nsock](size_t index, std::vector<SOCKET>::iterator it) -> bool {
                            nsock.sockfd = *it;
                            std::cout << "error." << std::endl;
                            socket_shutdown(&nsock, SHUT_RDWR);
                            socket_close(&nsock);
                            return true;
                        });
                    }
                }
            }
        }
    }

    std::cout << errstr() << std::endl;

    epoll_close(&ep);
    socket_close(&sock);
    
    return 0;
}