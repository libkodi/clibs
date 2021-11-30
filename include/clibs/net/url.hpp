#ifndef _CLIBS_URL_H_
#define _CLIBS_URL_H_ 1

#include <iostream>
#include <cstdlib>
#include "clibs/regexp.hpp"

namespace clibs {
    namespace net {
        /**
         * url结构体
         */
        typedef struct {
            std::string protocol;
            std::string host;
            int port;
            std::string path;
            std::string query;
            std::string hash;
        } url_t;

        /**
         * 解析url链接
         * @param  url         url_t
         * @param  url_content 链接文本
         * @return             true/false
         */
        bool url_parse(url_t* url, std::string url_content) {
            std::string port;

            CRegexp reg("^([a-zA-Z]+)://([^/:]+|)(:[\\d]+|)(/[^\\?#]+|)(\\?[^#]+|)(#.+|)$");
            CMatcher matcher = reg.matches(url_content);

            if (matcher.find()) {
                std::smatch smatch = matcher.result();
                url->protocol = smatch.format("$1");
                url->host = smatch.format("$2");
                port = std::regex_replace(smatch.format("$3"), std::regex(":"), "");
                url->path = smatch.format("$4");
                url->query = smatch.format("$5");
                url->hash = smatch.format("$6");
            } else {
                return false;
            }

            if (url->protocol == "") {
                return false;
            } else if (url->host == "") {
                return false;
            }

            if (port == "") {
                port = (url->protocol == "https" || url->protocol == "wss") ? "443" : url->protocol == "ftp" ? "21" : "80";
            }

            url->port = std::atoi(port.c_str());

            if (url->path == "") {
                url->path = "/";
            }

            return true;
        }
    }
}

#endif