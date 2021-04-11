#ifndef _EVENT_LOOP_THREAD_POOL_H_
#define _EVENT_LOOP_THREAD_POOL_H_
#include<sys/eventfd.h>
#include<vector>
#include<fcntl.h>
#include<iostream>
#include<assert.h>
#include<pthread.h>
#include"libco/co_routine.h"
#include"Eventloop.h"
#include"TimeRound.h"
using namespace std;
class EventLoopThreadPool
{
    public:
        EventLoopThreadPool(int _thread_num);
        int GetNextEventEpollFd();//轮询获得下一个分配的event_fd
        void start();
        ~EventLoopThreadPool();
    private:
        vector<int> event_fds;//用于通知对应的epoll来了新的连接，此时会将对应新连接的数据轮询写入到对应线程的epoll中
        int thread_num;
        int cur_event_epoll;
};
#endif