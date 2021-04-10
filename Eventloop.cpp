#include"Eventloop.h"
using namespace std;

EventLoop::EventLoop(int size):m_epoll(size)
{

}

void EventLoop::loop()
{
    while(true)
    {
        vector<SP_channel> event_fd = m_epoll.Poll();
    }
}
//专门用于接收连接的epoll
void* EventLoop::MainLoop(void* ptr)
{
    EventLoop* EVLoop = (EventLoop*)ptr;
    while(true)
    { 
        vector<SP_channel> event_fd = EVLoop->m_epoll.Poll();
    }
}
