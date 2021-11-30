#ifndef _CLIBS_REGEXP_H_
#define _CLIBS_REGEXP_H_ 1

#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <sstream>
#include <functional>

namespace clibs {
    /**
     * 一个匹配器封装
     */
    class CMatcher {
        private:
            std::sregex_iterator m_start, m_end;
        public:
            /**
             * 构造函数
             * @param reg std::regex
             * @param text 一个待处理文本的引用
             */
            CMatcher(std::regex *reg, std::string& text) {
                m_start = std::sregex_iterator(text.cbegin(), text.cend(), *reg);
            }

            /**
            * 获取匹配到的结果
            * @return std::smatch
            */
            std::smatch result() {
                std::smatch sm = *m_start;

                if (m_start != m_end) {
                    ++m_start;
                }

                return sm;
            }

            /**
             * 判断是否找到匹配
             * @return true/false
             */
            bool find() {
                return m_start->size() > 0;
            }
        };

    /**
     * 正则封装
     */
    class CRegexp {
        private:
            std::regex m_reg; 
        public:
            /**
             * 构造函数
             * @param regstr 正则表达式
             */
            CRegexp(std::string regstr) {
                m_reg = std::regex(regstr);
            }

            /**
             * 匹配一个文本
             * @param  text 待匹配文本
             * @return      true/false
             */
            bool test(std::string text) {
                return std::regex_search(text, m_reg);
            }

            /**
             * 完整匹配
             * @param  text 待匹配文本
             * @return      true/false
             */
            bool match(std::string text) {
                return std::regex_match(text, m_reg);
            }

            /**
             * 查找
             * @param  text 待匹配文本
             * @return      std::smatch
             */
            std::smatch search(std::string text) {
                std::smatch match_t;
                std::regex_search(text, match_t, m_reg);
                return match_t;
            }

            /**
             * 获取一个匹配器，用于全局&多次匹配
             * @param  text 待匹配文本
             * @return      CMatcher
             */
            CMatcher matches(std::string& text) {
                return CMatcher(&m_reg, text);
            }

            /**
             * 替换
             * @param  text     待替换文本
             * @param  repltext 替换文本
             * @return          替换后的结果文本
             */
            std::string replace(std::string text, std::string repltext) {
                return std::regex_replace(text, m_reg, repltext);
            }
                
            /**
             * 替换并使用回调函数处理结果
             * @param  text     待替换文本
             * @param  callback 处理结果的回调函数
             * @return          替换后的结果文本
             */
            std::string replace(std::string text, std::function<std::string(std::smatch)> callback) {
                std::stringstream res;
                size_t offset = 0, len;
                CMatcher m(&m_reg, text);

                while (m.find()) {
                    std::smatch sm = m.result();
                    len = sm.str().length();
                    res << text.substr(offset, sm.position(0) - offset) << callback(sm);
                    offset = sm.position(0) + len;
                }

                res << text.substr(offset);

                return res.str();
            }

            /**
             * 分割文本
             * @param text 待分割文本
             */
            std::vector<std::string> split(std::string text) {
                std::vector<std::string> res;
                CMatcher ms(&m_reg, text);
                size_t len, offset = 0;

                while (ms.find()) {
                    std::smatch sm = ms.result();
                    len = sm.str().length();
                    res.push_back(text.substr(offset, sm.position(0) - offset));
                    offset = sm.position(0) + len;
                }

                res.push_back(text.substr(offset));

                return res;
            }
    };


    /**
     * 简化调用封装
     */
    bool test(std::string reg, std::string text) {
        return std::regex_search(text, std::regex(reg));
    }

    bool match(std::string reg, std::string text) {
        return std::regex_match(text, std::regex(reg));
    }

    std::smatch search(std::string reg, std::string text) {
        CRegexp r(reg);
        return r.search(text);
    }

    std::vector<std::string> split(std::string reg, std::string text) {
        CRegexp r(reg);
        return r.split(text);
    }

    std::string replace(std::string reg, std::string text, std::string repl) {
        return std::regex_replace(text, std::regex(reg), repl);
    }

    std::string replace(std::string reg, std::string text, std::function<std::string(std::smatch)> callback) {
        CRegexp r(reg);
        return r.replace(text, callback);
    }
    
}

#endif