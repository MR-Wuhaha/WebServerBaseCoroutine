#include"Epoll.h"
using namespace std;

Epoll::Epoll():co_epoll(nullptr)
{
    co_epoll = co_get_epoll_ct();
    assert(co_epoll != nullptr);
}
