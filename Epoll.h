#ifndef _Epoll
#define _Epoll
#include<sys/epoll.h>
#include"channel.h"
#include<vector>
#include<assert.h>
#include<iostream>
#include<unordered_map>
#include"libco/co_routine.h"
#include"libco/co_epoll.h"
#include"libco/co_routine_inner.h"
using namespace std;
class Epoll
{
    private:
        stCoEpoll_t* co_epoll;
    public:
        Epoll();
        vector<SP_channel> Poll();
};

#endif