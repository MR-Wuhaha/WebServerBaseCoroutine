#include"EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(int _thread_num) : thread_num(_thread_num),
cur_event_epoll(0)
{
    event_fds.reserve(thread_num);
}

void EventLoopThreadPool::start()
{
    for(int i = 0;i < thread_num;i++)
    {
        int event_fd = eventfd(0,EFD_NONBLOCK);
        if(event_fd < 0)
        {
            printf("Invalid Eventfd\n");
            assert(0);
        }
        event_fds.push_back(event_fd);
        pthread_t pid;
        pthread_create(&pid,NULL,EventLoop::loop,&event_fds[i]);
    }
}

int EventLoopThreadPool::GetNextEventEpollFd()
{
    cur_event_epoll = (cur_event_epoll + 1) % thread_num;
    return event_fds[cur_event_epoll];
}

EventLoopThreadPool::~EventLoopThreadPool()
{

}