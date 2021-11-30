#ifndef _CLIBS_ARRAY_H_
#define _CLIBS_ARRAY_H_ 1

#include <iostream>
#include <vector>
#include <tuple>

namespace clibs {
    /**
     * 遍历vector
     */
    template<typename T, class Func>
    void array_foreach(std::vector<T>* arr, Func callback) {
        typename std::vector<T>::iterator it, end;

        if (arr->size() < 1) {
            return;
        } else {
            size_t index = 0;

            for (it = arr->begin(), end = arr->end(); it != end; it++) {
                if (!callback(index, it)) {
                    break;
                }

                index ++;
            }
        }
    }

    /**
     * 从vector中查找目标位置
     */
    template<typename T>
    ssize_t array_index(std::vector<T>* arr, T target) {
        ssize_t index = -1;

        array_foreach(arr, [&index, target](size_t i, typename std::vector<T>::iterator it) -> bool {
            if (*it == target) {
                index = i;
                return false;
            }

            return true;
        });

        return index;
    }

    /**
     * 遍历数组
     */
    template<typename T, class Func>
    void array_foreach(T* arr, size_t length, Func callback) {
        if (length < 1) {
            return;
        } else {
            for (size_t i = 0; i < length; i ++) {
                if (!callback(i, arr + i)) {
                    break;
                }
            }
        }
    }

    /**
     * 从数组中查找目标
     */
    template<typename T>
    ssize_t array_index(T* arr, size_t length, T target) {
        ssize_t index = -1;

        array_foreach(arr, length, [&index, target](size_t i, T* val) -> bool {
            if (*val == target) {
                index = i;
                return false;
            }

            return true;
        });

        return index;
    }

    template<class Func>
    void __foreach_args(Func fn, size_t index) {}

    template<class Func, typename T, typename... Args>
    void __foreach_args(Func fn, size_t index, T val, Args&&... args) {
        if (!fn(index, val)) {
            return;
        }
        
        __foreach_args(fn, index + 1, args...);
    }

    template<class Func, typename... Args>
    void foreach_args(Func fn, Args&&... args) {
        size_t index = 0;

        __foreach_args(fn, index, args...);
    }
}

#endif