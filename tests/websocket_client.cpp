#include <iostream>
#include <thread>
#include "clibs/error.hpp"
#include "clibs/net/websocket/websocket_client.hpp"

using namespace clibs::net::websocket;

void onopen(void* session) {
    std::cout << "opened" << std::endl;
}

void onerror(void* session, std::string error) {
    std::cout << "error: " << error << std::endl;
}

void onclose(void* session) {
    std::cout << "closed" << std::endl;
}

void onmessage(void* session, const char* data, ws_header_frame_t* pkg_info) {
    printf("data: %s\n", data);
}

CWebSocketClient client;

void connect_websocket() {
    std::string wsurl = "ws://localhost:8080/websocket";
    // std::string wsurl = "wss://localhost/websocket";

    client.onopen(onopen);
    client.onclose(onclose);
    client.onerror(onerror);
    client.onmessage(onmessage);

    client.run(wsurl);
}

int main(int argc, char const *argv[])
{
    std::thread th(connect_websocket);
    th.detach();

    std::string con;

    while (true) {
        std::cin >> con;

        if (con == "quit") {
            client.close();
            break;
        } else {
            client.send_text(true, con.c_str(), con.length());
        }
    }

    return 0;
}