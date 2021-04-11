#include"Epoll.h"
using namespace std;

Epoll::Epoll():co_epoll(nullptr)
{
    co_epoll = co_get_epoll_ct();
    assert(co_epoll != nullptr);
}

vector<SP_channel> Epoll::Poll()
{
    co_eventloop(co_get_epoll_ct(),0,0);
}