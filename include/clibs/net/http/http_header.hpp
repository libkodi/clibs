#ifndef _CLIBS_HTTP_HEADER_H_
#define _CLIBS_HTTP_HEADER_H_ 1

#include <iostream>
#include <sstream>
#include <map>
#include <vector>

namespace clibs {
    namespace net {
        namespace http {
            /**
             * http请求头封装
             */
            class CHttpHeader {
                private:
                    std::map<std::string, std::vector<std::string>> m_data;
                public:
                    CHttpHeader() {}

                    CHttpHeader(std::map<std::string, std::vector<std::string>> data) {
                        m_data = data;
                    }

                    /**
                     * 添加请求头信息
                     */
                    template<typename T>
                    void append(std::string name, T value) {
                        std::stringstream s_val;
                        s_val << value;
                        std::map<std::string, std::vector<std::string>>::iterator it;
                        
                        it = m_data.find(name);

                        if (it == m_data.end()) {
                            std::vector<std::string> field_array;
                            field_array.push_back(s_val.str());
                            m_data.insert(std::pair<std::string, std::vector<std::string>>(name, field_array));
                        } else {
                            it->second.push_back(s_val.str());
                        }
                    }

                    /**
                     * 判断是否包含请求头
                     * @param  name 请求头列名
                     * @return      true/false
                     */
                    bool contains(std::string name) {
                        std::map<std::string, std::vector<std::string>>::iterator it;
                        it = m_data.find(name);

                        return it != m_data.end();
                    }

                    /**
                     * 获取对应请求头信息的所有数据
                     */
                    std::vector<std::string> get_fields(std::string name) {
                        std::map<std::string, std::vector<std::string>>::iterator it;
                        it = m_data.find(name);

                        if (it == m_data.end()) {
                            return std::vector<std::string>();
                        } else {
                            return it->second;
                        }
                    }

                    /**
                     * 获取请求头信息的内容
                     * @param  name  请求头列名
                     * @param  index 下标
                     * @return       结果文本
                     */
                    std::string get(std::string name, int index) {
                        std::vector<std::string> field_array = get_fields(name);

                        if (index < field_array.size()) {
                            return field_array[index];
                        }

                        return "";
                    }

                    /**
                     * 获取请求头信息的内容
                     * @param  name  请求头列名
                     * @param  index 下标
                     */
                    std::string get(std::string name) {
                        return get(name, 0);
                    }

                    /**
                     * 拼接成请求头格式数据
                     * @return 拼接后的请求头
                     */
                    std::string str() {
                        std::stringstream ss;

                        std::map<std::string, std::vector<std::string>>::iterator map_iter, map_end;
                        std::vector<std::string>::iterator field_iter, field_end;

                        for (map_iter = m_data.begin(), map_end = m_data.end(); map_iter != map_end; map_iter++) {
                            for (field_iter = map_iter->second.begin(), field_end = map_iter->second.end(); field_iter != field_end; field_iter ++) {
                                ss << map_iter->first << ": " << *field_iter << "\r\n";
                            }
                        }

                        return ss.str();
                    }

                    /** 删除整个请求头信息 */
                    void remove(std::string name) {
                        std::map<std::string, std::vector<std::string>>::iterator it;

                        it = m_data.find(name);

                        if (it != m_data.end()) {
                            m_data.erase(it);
                        }
                    }

                    /** 删除请求头信息下的指定位置内容 */
                    void remove(std::string name, int index) {
                        std::map<std::string, std::vector<std::string>>::iterator it;

                        it = m_data.find(name);

                        if (it != m_data.end()) {
                            if (index < it->second.size()) {
                                it->second.erase(it->second.begin() + index);
                            }
                        }
                    }

                    std::map<std::string, std::vector<std::string>> data() {
                        return m_data;
                    }
            };
        }
    }
}

#endif