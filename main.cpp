#include<iostream>
#include"Epoll.h"
#include"channel.h"
#include"Eventloop.h"
#include"Task.h"
#include"ThreadPool.h"
#include"EventLoopThreadPool.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<cstring>
#include"otherFun.h"
#include <functional>
#include"Httpdata.h"
#include"Log.h"
#include<unistd.h>
using namespace std;
int main(int argv,char** argc)
{
    int epoll_thread_num = 1;
    if(argv == 2)
    {
        epoll_thread_num = atoi(argc[1]);
    }
    if(epoll_thread_num < 0 || epoll_thread_num > 10)
    {
        cerr<<"argment num error,enter number as the thread num..."<<endl;
        cout<<"process quit..."<<endl;
        return 0;
    }
    else
    {
        printf("%d epoll thread create...\n",epoll_thread_num);
    }
    //将当前进程变为守护进程在后台运行
    //int Defd = daemon(0,0);
    //assert(Defd == 0);


    //创建一个时间轮，参数为时间轮的最大定时时间间隔
#if 1
    TimeRound<channel> time_round = TimeRound<channel>(15);
    time_round.start();
    EventLoop::time_round = &time_round;//需要把时间轮的指针传递到每个EventLoop中，用于创建连接时找到时间论的信息
#endif
    //写日志对象
#if LOG_FLAG
    LogFile log_file("./ServerLog.txt");
    log_file.Start_Log();
    Log::log_file = &log_file;
#endif
    EventLoop *accept_epoll_loop = new EventLoop(100);//创建请求连接的epoll
    int lsfd = socket(AF_INET,SOCK_STREAM,0);
    if(lsfd < 0)
    {
        cout<<"create listen socket fail"<<endl;
        assert(0);
    }
    struct sockaddr_in Server_addr;
    memset(&Server_addr,0,sizeof(Server_addr));
    Server_addr.sin_family = AF_INET;
    Server_addr.sin_port = htons(80);
    Server_addr.sin_addr.s_addr = htonl((in_addr_t)0);
    int reuse = 1;
    assert(set_noblock(lsfd) > 0);
    setsockopt(lsfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int flag = bind(lsfd,(struct sockaddr*) &Server_addr,sizeof(Server_addr));
    if(flag < 0)
    {
        cout<<"bind listen socket fail"<<endl;
        assert(0);
    }
    listen(lsfd,1000);
    SP_channel Listen(new Httpdata(lsfd,Maccept,NULL,EventLoop::time_round));
    Listen->SetEventLoop(accept_epoll_loop);
    stCoRoutine_t* co = 0;
    co_create(&co,NULL,channel::CoroutineFun,&Listen);
    co_resume(co);
    //如果参数线程多于1，则创建一个epoll的线程池，把当前的线程作为创建连接的线程，其他线程处理请求
    EventLoopThreadPool event_loop_thread_pool(epoll_thread_num-1);
    if(epoll_thread_num > 1)
    {
        Listen->SetEventLoopThreadPool(&event_loop_thread_pool);
        event_loop_thread_pool.start();
    }
    EventLoop::MainLoop(accept_epoll_loop);
}
