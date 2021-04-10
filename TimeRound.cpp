#include"TimeRound.h"
#include <functional>
Lock_Guard::Lock_Guard(pthread_mutex_t& _Mutex):Mutex(_Mutex)
{
    pthread_mutex_lock(&Mutex);
}

void Lock_Guard::unlock()
{
    pthread_mutex_unlock(&Mutex);
}

Lock_Guard::~Lock_Guard()
{
    pthread_mutex_unlock(&Mutex);
}

void* start_fun(void* obj)
{
    ThreadFun* arg = static_cast<ThreadFun*>(obj);
    arg->StartFun();
}