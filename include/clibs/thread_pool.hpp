#ifndef _CLIBS_THREAD_POOL_H_
#define _CLIBS_THREAD_POOL_H_ 1

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <atomic>

namespace clibs {
    // 线程池结构体
    typedef struct {
        std::vector<std::thread> pool; // 线程池
        std::queue<std::function<void()>> tasks; // 任务队列
        std::atomic<bool> running; // 当前运行状态
        std::mutex lock; // 互斥锁
        unsigned long int max_queue_size; // 最大数量
        bool detach; // 是否关闭时分离线程
        unsigned int idel_thread; // 当前空闲线程
        std::condition_variable condition_variable; // 条件变量
    } thread_pool_t;

    /**
     * 初始化线程池
     * @param thread_pool    
     * @param pool_size      线程池运行的线程数量
     * @param max_queue_size 最大的队列数量
     * @param use_detach     是否关闭时分离线程
     */
    void thread_pool_init(thread_pool_t* thread_pool, unsigned int pool_size, unsigned long int max_queue_size, bool use_detach) {
        thread_pool->running = true;
        thread_pool->idel_thread = 0;
        thread_pool->detach = use_detach;
        thread_pool->max_queue_size = max_queue_size;

        // 初始化线程池
        for (int i = 0; i < pool_size; i ++) {
            // 使用emplace_back与匿名函数创建线程，避免函数退出后线程的引用丢失
            thread_pool->pool.emplace_back([thread_pool]() {
                while (thread_pool->running) {
                    std::function<void()> task_func;

                    // 创建一个代码块来处理上锁与解锁，避免影响后面的task_func运行和避免(task_func运行时)阻塞其它线程的运行
                    {
                        std::unique_lock<std::mutex> lock{ thread_pool->lock };
                        
                        thread_pool->condition_variable.wait(lock, [thread_pool]() {
                            return !thread_pool->running || !thread_pool->tasks.empty();
                        }); // 等待任务到来

                        if (!thread_pool->running) { // 如果已经退出
                            return;
                        }

                        if (thread_pool->tasks.empty()) { // 如果任务队列为空
                            continue;
                        }

                        // 从队列中获取任务回调
                        task_func = std::move(thread_pool->tasks.front());
                        thread_pool->tasks.pop();
                    }

                    thread_pool->idel_thread--;
                    task_func();
                    thread_pool->idel_thread++;
                }
            });

            thread_pool->idel_thread++;
        }
    }

    void thread_pool_init(thread_pool_t* thread_pool, unsigned int pool_size, unsigned long int max_queue_size) {
        thread_pool_init(thread_pool, pool_size, max_queue_size, false);
    }

    void thread_pool_init(thread_pool_t* thread_pool, unsigned int pool_size) {
        thread_pool_init(thread_pool, pool_size, 10240, false);
    }

    /**
     * 添加任务到线程池中, 支持多类型多变量
     */
    template<class F, class... Args>
    void thread_pool_add_task(thread_pool_t* thread_pool, F&& fn, Args&&... args) {
        auto task = std::bind(std::forward<F>(fn), std::forward<Args>(args)...);
        
        {
            std::unique_lock<std::mutex> lock{ thread_pool->lock };

            /** 如果队列已经满后就等待队列有空闲 */
            if (thread_pool->tasks.size() >= thread_pool->max_queue_size) {
                thread_pool->condition_variable.wait(lock, [thread_pool]() {
                    return !thread_pool->running || (thread_pool->tasks.size() < thread_pool->max_queue_size);
                });
            }

            thread_pool->tasks.emplace([task]() {
                task();
            });
        }
        
        thread_pool->condition_variable.notify_one();
    }

    /** 关闭线程池 */
    void thread_pool_shutdown(thread_pool_t* thread_pool) {
        thread_pool->running = false;

        thread_pool->condition_variable.notify_all();

        for (std::vector<std::thread>::iterator it = thread_pool->pool.begin(), end = thread_pool->pool.end(); it != end; it++) {
            if (thread_pool->detach) {
                it->detach();
            } else if (it->joinable()) {
                it->join();
            }
        }
    }

    /** 类形式调用的封装 */
    class CThreadPool {
        public:
            CThreadPool() {}

            CThreadPool(unsigned int pool_size) {
                thread_pool_init(&m_thread_pool, pool_size);
            }

            CThreadPool(unsigned int pool_size, unsigned long int max_queue_size) {
                thread_pool_init(&m_thread_pool, pool_size, max_queue_size);
            }

            CThreadPool(unsigned int pool_size, unsigned long int max_queue_size, bool use_detach) {
                thread_pool_init(&m_thread_pool, pool_size, max_queue_size, use_detach);
            }

            void init(unsigned int pool_size) {
                thread_pool_init(&m_thread_pool, pool_size);
            }

            ~CThreadPool() {
                thread_pool_shutdown(&m_thread_pool);
            }

            template<class F, class... Args>
            void add_task(F&& fn, Args&&... args) {
                thread_pool_add_task(&m_thread_pool, std::forward<F>(fn), std::forward<Args>(args)...);
            }

        protected:
            thread_pool_t m_thread_pool;
    };
}

#endif