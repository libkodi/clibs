#ifndef _CLIBS_WEBSOCKET_CLIENT_H_
#define _CLIBS_WEBSOCKET_CLIENT_H_ 1

#include <iostream>
#include "clibs/net/websocket/websocket_common.hpp"

namespace clibs {
    namespace net {
        namespace websocket {
            class CWebSocketClient : public CWebSocket {
                public:
                    /**
                     * 初始化
                     */
                    CWebSocketClient():CWebSocket() {
                        m_version = 13;
                        m_protocol = "";
                        m_handshake_key = websocket_random_key();
                        select_init(&m_select);
                    }

                    /** 设置版本号 */
                    void set_version(unsigned int version) {
                        m_version = version;
                    }

                    /** 设置子协议 */
                    void set_protocol(std::string protocol) {
                        m_protocol = protocol;
                    }

                    /** 运行websocket连接 */
                    void run(std::string ws_url) {
                        if (!url_parse(&m_url, ws_url)) {
                            m_websocket.error = "Invalid websocket link";
                            do_error_callback((void*)this, m_websocket.error);
                        }

                        if (m_url.protocol == "wss") {
                            m_websocket.is_ssl = true;
                        } else {
                            m_websocket.is_ssl = false;
                        }

                        if (websocket_connect(&m_websocket, &m_url, &m_select, &m_select_result)) {
                            if (websocket_send_handshake(&m_websocket, &m_url, m_handshake_key, m_version, m_protocol)) {
                                if (websocket_parse_response(&m_websocket, &m_fields, &m_select, &m_select_result)) {
                                    wait_message();
                                } else {
                                    do_error_callback((void*)this, m_websocket.error);
                                }
                            } else {
                                do_error_callback((void*)this, m_websocket.error);
                            }
                        } else {
                            do_error_callback((void*)this, m_websocket.error);
                        }
                    }

                    /**
                     * 发送数据封装
                     * @param  is_fin 
                     * @param  opcode 数据包类型
                     * @param  data   需要发送的数据
                     * @param  length 数据长度
                     * @return        true/false
                     */
                    bool send(bool is_fin, int opcode, const char* data, unsigned int length) {
                        return websocket_send(&m_websocket, is_fin, opcode, true, data, length);
                    }

                    bool send_text(bool is_fin, const char* data, unsigned int length) {
                        return send(is_fin, WS_TEXT, data, length);
                    }

                    bool send_binary(bool is_fin, const char* data, unsigned int length) {
                        return send(is_fin, WS_BINARY, data, length);
                    }

                    bool send_ping(bool is_fin, const char* data, unsigned int length) {
                        return send(is_fin, WS_PING, data, length);
                    }

                    bool send_pong(bool is_fin, const char* data, unsigned int length) {
                        return send(is_fin, WS_PONG, data, length);
                    }

                    bool send_text(const char* data, unsigned int length) {
                        return send_text(true, data, length);
                    }

                    bool send_binary(const char* data, unsigned int length) {
                        return send_binary(true, data, length);
                    }

                    bool send_ping(const char* data, unsigned int length) {
                        return send_ping(data, length);
                    }

                    bool send_pong(const char* data, unsigned int length) {
                        return send_pong(true, data, length);
                    }

                protected:
                    url_t m_url;
                    io::select_t m_select;
                    io::select_result_t m_select_result;
                    std::string m_handshake_key;
                    unsigned int m_version;
                    std::string m_protocol;
                    http::CHttpHeader m_fields;
                           
                protected:
                    /** 循环等待消息 */
                    void wait_message() {
                        do_open_callback((void*)this);

                        ws_header_frame_t pkg_info;

                        while (!m_is_quit) {
                            if (!websocket_get_package_info(&m_websocket, &pkg_info)) {
                                m_websocket.error = "Failed to read websocket package.";
                                do_error_callback((void*)this, m_websocket.error);

                                break;
                            } else {
                                if (pkg_info.opcode == WS_CLOSE) {
                                    break;
                                }

                                char buffer[pkg_info.extend_payload_len];
                                
                                int len = websocket_recv_must(&m_websocket, buffer, pkg_info.extend_payload_len);
                                buffer[pkg_info.extend_payload_len] = '\0';

                                if (pkg_info.mask) {
                                    websocket_unmask(pkg_info.mask_buffer, buffer, pkg_info.extend_payload_len);
                                }

                                do_message_callback((void*)this, buffer, &pkg_info);
                            }
                        }

                        do_close_callback((void*)this);
                    }
            };
        }
    }
}

#endif