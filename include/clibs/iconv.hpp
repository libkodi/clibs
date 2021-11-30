#ifndef _CLIBS_ICONV_H_
#define _CLIBS_ICONV_H_ 1
#include <iconv.h>
#include <iostream>
#include <malloc.h>
#include <cstring>
#include "clibs/string.hpp"

/**
 * iconv 编码转换的封装
 */
namespace clibs {
    /**
     * 指定iconv_t的转换封装
     * @param  cd     iconv_t
     * @param  inbuf  需要转换的文本
     * @param  inlen  转换文本长度
     * @param  outbuf 转换结果的输出
     * @param  outlen 尚未转换的文本大小
     * @return        true/false
     */
    bool iconv_convert(iconv_t cd, const char* inbuf, size_t* inlen, char* outbuf, size_t* outlen) {
        size_t inner_outlen = *outlen;
        char* inner_inbuf = (char*)inbuf;
        char* inner_outbuf = outbuf;
        int ret = iconv(cd, &inner_inbuf, inlen, &inner_outbuf, outlen);

        *(outbuf + (inner_outlen - *outlen)) = '\0';
        
        if (ret == -1) {
            return false;
        }

        return true;
    }

    /**
     * 指定编码类型转换
     * @param  from   输入编码
     * @param  to     输出编码
     * @param  inbuf  需要转换的文本
     * @param  inlen  转换文本长度
     * @param  outbuf 转换结果的输出
     * @param  outlen 尚未转换的文本大小
     * @return        true/false
     */
    bool iconv_convert(const char* from, const char* to, const char* inbuf, size_t* inlen, char* outbuf, size_t* outlen) {
        iconv_t cd = iconv_open(to, from);
        bool res = false;

        if (cd == (iconv_t)-1) {
            return res;
        } else {
            res = iconv_convert(cd, inbuf, inlen, outbuf, outlen);

            iconv_close(cd);
        }

        return true;
    }

    typedef struct {
        char* data;
        size_t length;
    } iconv_result_t;

    /**
     * 长文本转换封装
     * @param  from   输入编码
     * @param  to     输出编码
     * @param  data   需要转换的文本
     * @param  outstr 转换结果接收引用
     * @return        true/false
     */
    /**
     * 长文本转换封装
     * @param  from      输入编码
     * @param  to        输出编码
     * @param  inbuf     需要转换的文本
     * @param  inbuf_len 转换文本长度
     * @param  result    转换结果, iconv_result_t中的data为动态分配的内存，需要释放
     * @return           true/false
     */
    bool iconv_convert(const char* from, const char* to, const char* inbuf, size_t inbuf_len, iconv_result_t* result) {
        size_t step_convert_length = 1024, 
               inbuff_offset = 0, 
               outlen, 
               step_in_size = step_convert_length / 8, 
               step_in_length, 
               step_data_length,
               convert_result_size;
        char buff[step_convert_length];
        bool is_success;

        iconv_t cd = iconv_open(to, from);

        if (cd == (iconv_t)-1) {
            return false;
        } else {
            // 初始化一个空内存
            result->data = (char*)calloc(0, sizeof(char));
            result->length = 0;

            if (result->data == NULL) {
                return false;
            }

            /**
             * 循环分段转换
             */
            while (inbuff_offset < inbuf_len) {
                memset(buff, 0, step_convert_length);
                outlen = step_convert_length;

                /**
                 * 计算当前需要转换的大小
                 */
                step_data_length = inbuf_len - inbuff_offset;

                if (step_data_length > step_in_size) {
                    step_data_length = step_in_size;
                }

                step_in_length = step_data_length;

                /**
                 * 复制分段文本内存
                 */
                char temp_inbuff[step_data_length];
                memcpy(temp_inbuff, inbuf + inbuff_offset, step_data_length);

                // 执行转换
                is_success = iconv_convert(cd, (const char*)temp_inbuff, &step_in_length, buff, &outlen);
                
                // 转换后的数据大小
                convert_result_size = step_convert_length - outlen;
                // 被成功转换的数据大小
                inbuff_offset += step_data_length - step_in_length;

                // 转换失败并且转换成功字节数为0
                if (!is_success && convert_result_size == step_convert_length) {
                    iconv_close(cd);
                    return false;
                }
                
                // 重新分配内存
                char* new_result_data_ptr = (char*)realloc(result->data, (result->length + convert_result_size) * sizeof(char));

                if (new_result_data_ptr == NULL) {
                    return false;
                } else {
                    // 复制转换结果
                    result->data = new_result_data_ptr;
                    memcpy(result->data + result->length, buff, convert_result_size);
                    result->length += convert_result_size;
                }
            }

            iconv_close(cd);
        }

        return true;    
    }

    /**
     * 释放iconv_result_t分配的动态内存
     */
    void iconv_free_result(iconv_result_t* result) {
        if (result->data != NULL) {
            free(result->data);
        }
    }
}

#endif