#ifndef _CLIBS_HTTP_READER_H_
#define _CLIBS_HTTP_READER_H_ 1

#include <iostream>
#include <map>
#include <vector>
#include "clibs/net/socket.hpp"
#include "clibs/net/sslsocket.hpp"
#include "clibs/io/select.hpp"
#include "clibs/net/http/http_header.hpp"

namespace clibs {
    namespace net {
        namespace http {
            typedef std::map<std::string, std::vector<std::string>> http_header;

            /**
             * 读取http包依赖的结构体
             */
            typedef struct {
                bool final; // 是否读取完结
                bool parsed_header;

                /**
                 * response
                 */
                int status_code;
                std::string error;

                /**
                 * request
                 */
                std::string url;
                std::string method;

                unsigned long long int content_length;
                bool chunked;
                unsigned long int chunked_package_size;
                unsigned long long int readed_size;

                socket_t *socket;
                ssl_socket_t *ssl_socket;
                io::select_t *select;
                io::select_result_t *select_result;

                unsigned int read_timeout;
            } http_reader_t;

            /**
             * 初始化读取器
             * @param reader   http_reader_t
             * @param sock     需要绑定的socket_
             * @param ssl_sock 需要绑定的ssl_socket_t, 如果连接的是https
             */
            void reader_init(http_reader_t* reader, socket_t* sock, ssl_socket_t* ssl_sock) {
                reader->status_code = 0;
                reader->final = false;
                reader->chunked = false;
                reader->content_length = 0;
                reader->readed_size = 0;
                reader->chunked_package_size = 0;
                reader->socket = sock;
                reader->ssl_socket = ssl_sock;
                reader->select = NULL;
                reader->select_result = NULL;
                reader->parsed_header = false;
            }

            /**
             * 绑定select信息
             * @param reader  http_reader_t
             * @param s       select_t
             * @param result  select_result_t
             * @param timeout 读取超时时间
             */
            void reader_set_select(http_reader_t* reader, io::select_t* s, io::select_result_t* result, unsigned int timeout) {
                reader->select = s;
                reader->select_result = result;
                reader->read_timeout = timeout;
            }

            /**
             * 执行select操作
             * @param  reader select
             * @return        true/false
             */
            bool reader_doselect(http_reader_t* reader) {
                if (reader->select == NULL || reader->select_result == NULL) {
                    return true;
                }

                if (reader->ssl_socket != NULL) {
                    int size = socket_ssl_pending(reader->ssl_socket);

                    if (size < 0) {
                        return false;
                    } else if (size > 0) {
                        return true;
                    }
                }

                if (reader->socket == NULL) {
                    return false;
                } else {
                    if (io::select_wait(reader->select, reader->select_result, reader->read_timeout) <= 0) {
                        return false;
                    } else {
                        if (io::select_find(&reader->select_result->exception, reader->socket->sockfd) != reader->select_result->exception.end()) {
                            return false;
                        }

                        if (io::select_find(&reader->select_result->read, reader->socket->sockfd) == reader->select_result->read.end()) {
                            return false;
                        }
                    }
                }

                return true;
            }

            /** 接收一个字符 */
            int reader_recv(http_reader_t* reader) {
                if (!reader_doselect(reader)) {
                    return -1;
                }

                if (reader->ssl_socket != NULL) {
                    return socket_ssl_recv(reader->ssl_socket);
                } else {
                    return socket_recv(reader->socket);
                }
            }

            /** 接收数据 */
            int reader_recv(http_reader_t* reader, char* buffer, int length) {
                if (!reader_doselect(reader)) {
                    return -1;
                }

                if (reader->ssl_socket != NULL) {
                    return socket_ssl_recv(reader->ssl_socket, buffer, length);
                } else {
                    return socket_recv(reader->socket, buffer, length);
                }
            }

            /** 接收固定长度数据 */
            int reader_recv_must(http_reader_t* reader, char* buffer, int length) {
                return __socket_recv_must(length, [reader, buffer, length](int offset) -> int {
                    return reader_recv(reader, buffer + offset, length - offset);
                });
            }

            /**
             * 接收单行数据
             * @param  reader 
             * @param  buffer 单行数据的输出
             * @return        读取到的数据长度
             */
            int reader_recvline(http_reader_t* reader, char* buffer) {
                return __socket_readline(buffer, [reader]() -> int {
                    return reader_recv(reader);
                });
            }

            /**
             * 解析http请求头
             * @param  reader   
             * @param  pHeaders 用来接收请求头参数的map
             * @return          true/false
             */
            bool reader_parse_header(http_reader_t* reader, CHttpHeader* pHeaders) {
                std::string strHeader;
                int nFlags = 0, nChar, nTopLineLength;
                char strTopLine[200];

                // 读取首行
                nTopLineLength = reader_recvline(reader, strTopLine);

                if (nTopLineLength <= 0) {
                    return false;
                } else {
                    strTopLine[nTopLineLength] = '\0';
                }

                /** 读取完整请求头 */
                while (nFlags < 4) {
                    nChar = reader_recv(reader);

                    if (nChar == -1) {
                        return false;
                    } else {
                        if (nChar == 10 || nChar == 13) {
                            nFlags ++;
                        } else {
                            nFlags = 0;
                        }

                        strHeader += (char)nChar;
                    }
                }

                std::string strFirstLine = strTopLine;

                // 如果是请求数据
                if (test("^[A-Z]+\\s.+\\sHTTP/1\\.\\d", strFirstLine)) {
                    CRegexp reg("^([A-Z]+)\\s(.+)\\sHTTP/1\\.\\d");
                    CMatcher m = reg.matches(strFirstLine);
                    std::smatch sm = m.result();
                    reader->url = sm.str(2);
                    reader->method = sm.str(1);
                }
                // 如果是响应数据
                 else if (test("^HTTP/1\\.\\d\\s\\d+\\s.*", strFirstLine)) {
                    CRegexp reg("^HTTP/1\\.\\d\\s(\\d+)\\s(.*)");
                    CMatcher m = reg.matches(strFirstLine);
                    std::smatch sm = m.result();
                    reader->status_code = std::atoi(sm.str(1).c_str());
                    reader->error = sm.str(2);
                } else {
                    return false;
                }

                /**
                 * 解析请求头中的参数
                 */
                CRegexp cReg("([\\w-]+):\\s(.+)");
                std::string strKey, strValue;
                CMatcher cMatches = cReg.matches(strHeader);

                while (cMatches.find()) {
                    std::smatch cSmatch = cMatches.result();

                    strKey = cSmatch.str(1);
                    strValue = cSmatch.str(2);
                    pHeaders->append(strKey, strValue);
                }

                /**
                 * 判断body的长度
                 */
                std::string content_length_str = pHeaders->get("Content-Length");

                if (content_length_str != "") {
                    reader->content_length = std::atol(content_length_str.c_str());

                    if (reader->content_length == 0) {
                        reader->final = true;
                    }
                } else {
                    std::string transfer_encoding = pHeaders->get("Transfer-Encoding");

                    if (transfer_encoding != "" && transfer_encoding == "chunked") {
                        reader->chunked = true;
                    } else {
                        reader->final = true;
                    }
                }

                reader->parsed_header = true;

                return true;
            }

            /** 读取http的body内容的封装 */
            int reader_read(http_reader_t* pReader, char* buffer, int length) {
                if (pReader->final || !pReader->parsed_header) {
                    buffer[0] = '\0';
                    return -1;
                }

                int len;

                // 如果请求头包含transfer-coding: chuncked
                if (pReader->chunked) {
                    // 读取分包的大小
                    if (pReader->chunked_package_size == 0) {
                        char strPkgSize[100];
                        int nHexLen = reader_recvline(pReader, strPkgSize);

                        if (nHexLen <= 0) {
                            buffer[0] = '\0';
                            return nHexLen;
                        } else {
                            strPkgSize[nHexLen] = '\0';
                        }

                        // hex to int
                        std::stringstream strHex;
                        strHex << std::hex << strPkgSize;
                        strHex >> pReader->chunked_package_size;

                        if (pReader->chunked_package_size == 0) {
                            pReader->final = true;
                            return -1;
                        }
                    }

                    // 计算可以读取的数据大小
                    unsigned long long int read_size = pReader->chunked_package_size > length ? length : pReader->chunked_package_size;

                    len = reader_recv(pReader, buffer, read_size);

                    pReader->readed_size += (len > 0 ? len : 0);

                    if (len < 0) {
                        pReader->final = true;
                        return -1;
                    }

                    pReader->chunked_package_size -= (len > 0) ? len : 0;

                    if (pReader->chunked_package_size == 0) { // 如果当前包读取完毕，清除末尾的\r\n
                        char strTemp[2];
                        reader_recv_must(pReader, strTemp, 2);
                    }

                    return len;
                } else { // 请求头中有指定content-length
                    if (pReader->readed_size < pReader->content_length) {
                        len = reader_recv(pReader, buffer, length);

                        if (len <= 0) {
                            pReader->final = true;
                        } else {
                            pReader->readed_size += len;

                            if (pReader->readed_size >= pReader->content_length) {
                                pReader->final = true;
                            }
                        }

                        return len;
                    }
                }

                return -1;
            }

            /** 读取完整的http_body */
            std::string reader_full_read(http_reader_t* pReader) {
                std::string res;
                char buff[1024];
                int len;

                while (!pReader->final) {
                    memset(buff, 0, 1024);
                    len = reader_read(pReader, buff, 1024);

                    if (len == -1) {
                        break;
                    } else {
                        buff[len] = '\0';
                    }

                    res += buff;
                }

                return res;
            }

            /** 从body中读取一个字符 */
            int reader_read(http_reader_t* reader) {
                return __socket_read_char([reader](char* buffer) -> int {
                    return reader_read(reader, buffer, 1);
                });
            }

            /** 从body中读取一行 */
            int reader_readline(http_reader_t* reader, char* buffer) {
                return __socket_readline(buffer, [reader]() -> int {
                    return reader_read(reader);
                });  
            }

            /** 从body中读取固定长度数据 */
            int reader_read_must(http_reader_t* reader, char* buffer, int length) {
                return __socket_recv_must(length, [reader, buffer, length](int offset) -> int {
                    return reader_read(reader, buffer + offset, length - offset);
                });
            }
        }
    }
}

#endif