#ifndef _EVENTLOOP
#define _EVENTLOOP
#include<iostream>
#include<vector>
#include"ThreadPool.h"
#include"Epoll.h"
#include"TimeRound.h"
class EventLoop
{
    public:
        static TimeRound<channel> *time_round;
        Epoll m_epoll;
        EventLoop(int);
        static void* loop(void*);
        static void* MainLoop(void*);
        void AddWaitForFreeItem(stCoRoutine_t* co);
        static int FreeCo(void*);
    private:
        int event_fd;//当前线程对应的event_fd
        vector<stCoRoutine_t*> vecWaitforfree;//该线程中的主协程被唤醒是需要回收运行结束的协程资源
};

#endif
