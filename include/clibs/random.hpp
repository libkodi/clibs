#ifndef _CLIBS_RANDOM_H_
#define _CLIBS_RANDOM_H_ 1

#include <iostream>
#include <random>
#include <ctime>

namespace clibs {
    /**
     * 随机数封装
     */
    class CRandom {
        protected:
            std::default_random_engine m_engine;
        public:
            /**
             * 初始化随机种子
             */
            CRandom() {
                m_engine.seed(time(NULL));
            }

            /**
             * 获取一个随机数
             */
            unsigned int next() {
                return m_engine();
            }

            /**
             * 从指定范围内返回一个随机数
             */
            template <typename T>
            T next_int(T start, T end) {
                std::uniform_int_distribution<T> u(start, end);

                return u(m_engine);
            }

            /**
             * 从指定范围内返回一个带小数点的随机数
             */
            template <typename T>
            T next_float(T start, T end) {
                std::uniform_real_distribution<T> u(start, end);

                return u(m_engine);
            }
    };
}

#endif