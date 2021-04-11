#include"Eventloop.h"
#include"Httpdata.h"
using namespace std;

EventLoop::EventLoop(int _event_fd):m_epoll(),event_fd(_event_fd)
{

}
/*
用于处理数据的线程Epoll，每个线程都先创建一个EventFd，当主线程收到新的连接时，通过向该EventFd发送数据
唤醒该线程中poll的协程，创建新的连接并将新的连接以协程的方式在该线程中运行
*/
void* EventLoop::loop(void* ptr)
{
    co_enable_hook_sys();
    int event_fd = *((int*)ptr);
    EventLoop event_loop(event_fd);
    stCoRoutine_t* co = 0;
    SP_channel event_channel = SP_channel(new Httpdata(event_fd,sysread,writen,time_round));
    co_create(&co,NULL,channel::HandleNewConnectCoroutineFun,&event_channel);
    co_resume(co);
    while(true)
    {
        cout<<"Epoll Wait Eventfd : "<<event_fd<<endl;
        event_loop.m_epoll.Poll();
    }
}
//专门用于接收连接的epoll
void* EventLoop::MainLoop(void* ptr)
{
    EventLoop* EVLoop = (EventLoop*)ptr;
    while(true)
    {
        cout<<"Epoll Wait MainLoop"<<endl;
        vector<SP_channel> event_fd = EVLoop->m_epoll.Poll();
    }
}
 
TimeRound<channel>* EventLoop::time_round = nullptr;
