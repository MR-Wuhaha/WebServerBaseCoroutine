#include"otherFun.h"
#include"Epoll.h"
#include"channel.h"
#include"Log.h"
#include"EventLoopThreadPool.h"
using namespace std;
int co_accept(int fd, struct sockaddr *addr, socklen_t *len );
int Maccept(SP_channel _channel,char *buff,int length)
{
    while(true)
    {
        struct sockaddr_in Client_addr;
        memset(&Client_addr,0,sizeof(Client_addr));
        socklen_t Client_addr_len = sizeof(Client_addr);
        int connfd = co_accept(_channel->fd,(struct sockaddr*)&Client_addr,&Client_addr_len);
        if(connfd > 0)
        {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&Client_addr.sin_addr.s_addr,client_ip,sizeof(client_ip));
#if LOG_FLAG
            LOG<<"accept connect from : " + string(client_ip) + " port:" + to_string(Client_addr.sin_port) << " ClientFd:" << connfd;
#endif
            //cout<<"accept connect from : "<<client_ip << " port:" << Client_addr.sin_port <<endl;
            /*
            如果只有一条线程在运行，则当前的线程作为处理连接数据和接收新连接的复用线程
            如果有多条线程在运行，则当前线程只用于建立连接，而将新建立的连接轮询分发到其他线程处理
            */
            if(_channel->event_loop_thread_pool == nullptr)
            {
                _channel->Add_New_Connect(connfd);
            }
            else
            {
                uint64_t new_connect_msg = connfd;
                int distribute_fd = _channel->event_loop_thread_pool->GetNextEventEpollFd();
                //cout<<"Distribute Epool Thread EventFd : "<< distribute_fd<<endl;
                int ret = write(distribute_fd,&new_connect_msg,sizeof(uint64_t));
                if(ret != sizeof(uint64_t))
                {
                    if(ret < 0)
                    {
                        if(errno == EAGAIN)
                        {
                            cout<<"operator should be block"<<endl;
                        }
                        else if(errno == EINTR)
                        {
                            cout<<"operator has been interrupt"<<endl;
                        }
                        else
                        {
                            cout<<"Unknow error"<<endl;
                        }
                    }
                    printf("send msg length %d, should send %d, new_connect_msg send fail!\n",ret,sizeof(uint64_t));
                    assert(0);
                }
            }
        }
        else
        {
            if(errno == EAGAIN)
            {
                pollfd* pf = (struct pollfd*)calloc( 1,sizeof(struct pollfd));
                pf[0].fd = _channel->fd;
                pf[0].events = (POLLIN | POLLERR | POLLHUP);
                int ret = poll(pf,1,5000);
                if(ret != 0 && pf[0].revents != POLLIN)
                {
                    break;
                }
            }
        }
    }
    return -1;
}
int readn(SP_channel _channel,char *buff,int length)
{
    memset(buff,0,length);
    int rlen = recv(_channel->fd,buff,length,0);
    if(rlen == 0)
    {
        //cout<<"client fd: "<<_channel->fd<<" closed or exception happend!"<<endl;
#if LOG_FLAG
        LOG<<"client fd: "+to_string(_channel->fd) + " closed or exception happend!";
#endif        
        //对端关闭连接或者连接异常断开
        _channel->handle_close();
        return -1;
    }
    if(rlen < 0)
    {
        return 0;
    }
    return rlen;
}
/*
用于通知EventFd的写数据函数，由于不是通过co_accrpt创建的fd，所以没有被系统hook住
需要设置为非阻塞的并在读不到数据时手动poll让出协程
*/
int sysread(SP_channel _channel,char *buff,int length)
{
    memset(buff,0,length);
    int rlen = 0;
    int readlength = 0;
    while(true)
    {
        rlen = read(_channel->fd,buff,length);
        if(rlen == 0)
        {
            return -1;
        }
        if(rlen < 0)
        {
            if(errno == EAGAIN && readlength == 0)
            {
                pollfd* pf = (struct pollfd*)calloc( 1,sizeof(struct pollfd));
                pf[0].fd = _channel->fd;
                pf[0].events = (POLLIN | POLLERR | POLLHUP);
                int ret = poll(pf,1,1000*60*5);
            }
            else if(errno == EAGAIN && readlength != 0)
            {
                break;
            }
            else if(errno == EINTR)
            {
                continue;
            }
        }
        else
        {
            readlength += rlen;
        }
    }
    return readlength;
}

int writen(SP_channel _channel,char *buff,int length)
{
    int wlen = send(_channel->fd,buff,length,0);
    if(wlen == 0)
    {
        //cout<<"client fd: "<<_channel->fd<<" closed or exception happend!"<<endl;
#if LOG_FLAG
        LOG<<"client fd: "+to_string(_channel->fd) + " closed or exception happend!";
#endif        
        //对端关闭连接或者连接异常断开
        _channel->handle_close();
        return -1;
    }
    if(wlen < 0)
    {
        return 0;
    }
    return wlen;
}
int set_noblock(int sockfd)
{
    if(sockfd < 0)
    {
        cout<<"sockfd is invailed"<<endl;
        return -1;
    }
    int flags = fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,flags | O_NONBLOCK);
    return 1;
}