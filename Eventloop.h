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
    private:
        int event_fd;//当前线程对应的event_fd
};

#endif
