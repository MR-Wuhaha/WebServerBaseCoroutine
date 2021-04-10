#include<iostream>
#include"Epoll.h"
#include"channel.h"
#include"Eventloop.h"
#include"Task.h"
#include"ThreadPool.h"
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
int main()
{
    //将当前进程变为守护进程在后台运行
    //int Defd = daemon(0,0);
    //assert(Defd == 0);


    //创建一个时间轮，参数为时间轮的最大定时时间间隔
    TimeRound<channel> time_round = TimeRound<channel>(15);
    time_round.start();

    //写日志对象
    LogFile log_file("./ServerLog.txt");
    log_file.Start_Log();
    Log::log_file = &log_file;

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
    SP_channel Listen(new Httpdata(lsfd,Maccept,NULL,&time_round));
    stCoRoutine_t* co = 0;
    co_create(&co,NULL,channel::CoroutineFun,&Listen);
    co_resume(co);
    EventLoop::MainLoop(accept_epoll_loop);
    // pthread_t MainEpollLoop_id;
    // pthread_create(&MainEpollLoop_id,NULL,EventLoop::MainLoop,accept_epoll_loop);
    // pthread_setname_np(MainEpollLoop_id,"MLThread");
    // cout<<"MainLoopThread Start"<<endl;
    // pthread_join(MainEpollLoop_id,NULL);
    //ThreadPool mThreadPool(4,100);
    //mThreadPool.start_thread();
}
