#ifndef _CHANNEL
#define _CHANNEL
#include<iostream>
#include<sys/socket.h>
#include<functional>
#include<memory>
#include"TimeRound.h"
#include<sys/epoll.h>
#include"otherFun.h"
#include"Log.h"
#include"libco/co_routine.h"
class Epoll;
class channel;
class Timer_node;
class EventLoopThreadPool;
typedef std::shared_ptr<channel> SP_channel;
typedef int(*Handle)(SP_channel,char*,int);
class channel:public enable_shared_from_this<channel>
{
    protected:
        int fd;
        Handle read;
        Handle write;
        char* read_buff;
        char* write_buff;
        int read_length;
        int write_length;
        int max_read_write_buff_length;
        int length;
        TimeRound<channel>* time_round;
        std::weak_ptr<TimeRoundItem<channel>> wp_time_round_item;
        //用于分配新连接的线程池，每个线程池中运行多个协程
        EventLoopThreadPool* event_loop_thread_pool;
    public:
        friend class Epoll;
        channel(int _fd,Handle _read,Handle _write,TimeRound<channel>* _time_round);
        virtual ~channel();
        virtual int handle_event();
        virtual int handle_close();
        virtual void HandleRead();
        virtual void HandleWrite();
        static void* CoroutineFun(void*);
        static void* HandleNewConnectCoroutineFun(void*);
        bool Get_KeepAlive_State();
        void SetEventLoopThreadPool(EventLoopThreadPool*);
        void Close();
        friend int Maccept(SP_channel _channel,char *buff,int length);
        friend int readn(SP_channel _channel,char *buff,int length);
        friend int writen(SP_channel _channel,char *buff,int length);
        friend int sysread(SP_channel _channel,char *buff,int length);
        virtual void Add_New_Connect(int);
        int get_fd();
        virtual void SeparateTimer();
};
#endif