# create httpclient
add_executable(httpclient httpclient.cpp)
target_link_libraries(httpclient ssl crypto)

if(WIN32)
target_link_libraries(httpclient ws2_32)
endif()

# url
add_executable(url url.cpp)

# sockaddr
add_executable(sockaddr sockaddr.cpp)

if(WIN32)
target_link_libraries(sockaddr ws2_32)
endif()

# string
add_executable(string string.cpp)

# ssl_tcp_client
add_executable(ssl_tcp_client ssl_tcp_client.cpp)
target_link_libraries(ssl_tcp_client ssl crypto)

if(WIN32)
target_link_libraries(ssl_tcp_client ws2_32)
endif()

# tcp_client
add_executable(tcp_client tcp_client.cpp)

if(WIN32)
target_link_libraries(tcp_client ws2_32)
endif()

# tcp_server
add_executable(tcp_server tcp_server.cpp)

if(WIN32)
target_link_libraries(tcp_server ws2_32)
endif()

# udp_client
add_executable(udp_client udp_client.cpp)

if(WIN32)
target_link_libraries(udp_client ws2_32)
endif()

# udp_server
add_executable(udp_server udp_server.cpp)

if(WIN32)
target_link_libraries(udp_server ws2_32)
endif()

# regexp
add_executable(regexp regexp.cpp)

# coding
add_executable(coding coding.cpp)

if(APPLE OR WIN32)
	target_link_libraries(coding iconv)
endif()

# create websocket_client
add_executable(websocket_client websocket_client.cpp)
target_link_libraries(websocket_client ssl crypto pthread)

if(WIN32)
target_link_libraries(websocket_client ws2_32)
endif()

# create random
add_executable(random random.cpp)

#create base64
add_executable(base64 base64.cpp)
target_link_libraries(base64 ssl crypto)

# create sha1
add_executable(sha1 sha1.cpp)
target_link_libraries(sha1 crypto)

# create select
add_executable(select select.cpp)

if(WIN32)
target_link_libraries(select ws2_32)
endif()

#create rsa_sign_test
add_executable(rsa_test rsa_test.cpp)
target_link_libraries(rsa_test crypto)

# create thread
add_executable(thread thread.cpp)

target_link_libraries(thread pthread)

#create os_test
add_executable(os_test os_test.cpp)

if(LINUX)
#create epoll
add_executable(epoll epoll.cpp)
endif()

#create array_test
add_executable(array_test array_test.cpp)

# create sqlite3
add_executable(sqlite3_test sqlite3.cpp)
target_link_libraries(sqlite3_test sqlite3)

# create args
add_executable(args_test args.cpp)

# create uuid
add_executable(uuid_test uuid.cpp)
target_link_libraries(uuid_test uuid)