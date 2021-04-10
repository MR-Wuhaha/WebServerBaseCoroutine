#ifndef _THREADPOOL
#define _THREADPOOL
#include<pthread.h>
#include<vector>
#include"Task.h"
#include"Eventloop.h"
using namespace std;
class ThreadPool
{
    private:
        static Task* task_list;
        static int max_thread_num;
        static int max_task_num;
        static vector<pthread_t> threads;
    public:
        friend class EventLoop;
        ThreadPool(int thread_num,int task_num);
        static void* loop(void* args);
        void start_thread();
        ~ThreadPool();
};
#endif