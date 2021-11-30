#ifndef _CLIBS_BASE64_H_
#define _CLIBS_BASE64_H_ 1
#include <iostream>
#include <cstring>
#include <cmath>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

namespace clibs {
   namespace encrypt {
      /** 计算base64后的大小 */
      size_t base64_encode_size(size_t length) {
         return std::ceil((double)length / 3.0) * 4;
      }

      /** 计算解base64后的大小 */
      size_t base64_decode_size(size_t length) {
         return (length / 4) * 3;
      }

      /**
       * base64加密
       * @param  text    需要加密文本
       * @param  textlen 文本长度
       * @return         加密结果字符串
       */
      void base64_encode(const char* text, size_t textlen, char* outbuf, size_t* outlen) {
         if (text == NULL || textlen < 1) {
            return;
         }

         BIO *b64, *bio;
         BUF_MEM *bptr;
         
         b64 = BIO_new(BIO_f_base64());
         bio = BIO_new(BIO_s_mem());
         bio = BIO_push(b64, bio);

         BIO_write(bio, text, textlen);
         BIO_flush(bio);
         BIO_get_mem_ptr(bio, &bptr);

         *outlen = bptr->length;
         memcpy(outbuf, bptr->data, bptr->length);
         outbuf[bptr->length] = '\0';

         BIO_free_all(bio);

         for (size_t i = 0, j = 0, len = *outlen; i < len; i ++) {
            if (*(outbuf + i) != '\n') {
               *(outbuf + j) = *(outbuf + i);
               j++;
            } else {
               *outlen -= 1;
            }
         }

         outbuf[*outlen] = '\0';
      }

      /**
       * base64解密
       * @param  text   解密数据
       * @param  length 数据长度
       * @return        解密结果
       */
      void base64_decode(const char* text, size_t length, char* outbuf, size_t* outlen) {
         if (text == NULL || length < 1) {
            return;
         }

         BIO *b64, *bio;

         b64 = BIO_new(BIO_f_base64());
         BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

         bio = BIO_new_mem_buf(text, length);
         bio = BIO_push(b64, bio);

         size_t size = BIO_read(bio, outbuf, length);
         outbuf[size] = '\0';
         *outlen = size;
         BIO_free_all(bio);
      }
   }
}   

#endif