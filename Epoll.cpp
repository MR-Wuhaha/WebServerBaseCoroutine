#include"Epoll.h"
using namespace std;

Epoll::Epoll(int _size):co_epoll(nullptr)
{
    co_epoll = co_get_epoll_ct();
}

vector<SP_channel> Epoll::Poll()
{
    co_eventloop(co_get_epoll_ct(),0,0);
}