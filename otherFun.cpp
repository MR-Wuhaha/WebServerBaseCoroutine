#include"otherFun.h"
#include"Epoll.h"
#include"channel.h"
#include"Log.h"
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
            fcntl( connfd, F_SETFL, fcntl(connfd, F_GETFL,0 ) );//协程需要将当前的fd设置为noblock，而不是由用户自己设置，通过hook的fcntl设置
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&Client_addr.sin_addr.s_addr,client_ip,sizeof(client_ip));
            LOG<<"accept connect from : " + string(client_ip) + " port:" + to_string(Client_addr.sin_port) << " ClientFd:" << connfd;
            //cout<<"accept connect from : "<<client_ip << " port:" << Client_addr.sin_port <<endl;
            _channel->Add_New_Connect(connfd);
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
        LOG<<"client fd: "+to_string(_channel->fd) + " closed or exception happend!";
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
int writen(SP_channel _channel,char *buff,int length)
{
    int wlen = send(_channel->fd,buff,length,0);
    if(wlen == 0)
    {
        //cout<<"client fd: "<<_channel->fd<<" closed or exception happend!"<<endl;
        LOG<<"client fd: "+to_string(_channel->fd) + " closed or exception happend!";
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