#include <iostream>
#include <unistd.h>
#include "clibs/thread_pool.hpp"

class A {
public:
    void run(int val) {
        std::cout << "Thread-" << std::this_thread::get_id() << ": " << val << std::endl;
    }
};

int main()
{
    A a;
    clibs::CThreadPool th(2);

    for (int i = 0, len = 100; i < len; i ++) {
        th.add_task(&A::run, &a, i);
    }

    sleep(2);

    return 0;
}