#include"channel.h"
#include"Epoll.h"
using namespace std;
channel::channel(int _fd,Handle _read,Handle _write,TimeRound<channel>* _time_round):fd(_fd),write(_write),read(_read),time_round(_time_round)
{
    wp_time_round_item = std::weak_ptr<TimeRoundItem<channel>>();
    max_read_write_buff_length = 4096;
    read_buff = new char[max_read_write_buff_length];
    write_buff = new char[max_read_write_buff_length];
    read_length = 0;
    write_length = 0;
}
int channel::handle_event()
{
    HandleRead();
    return 0;
}

void* channel::CoroutineFun(void* ptr)
{
    co_enable_hook_sys();
    SP_channel _channel = *((SP_channel*)(ptr));
    while(true)
    {
        _channel->handle_event();
        //客户端主动关闭
        if(_channel->read_length < 0 || _channel->write_length < 0)
        {
            break;
        }
        if(!_channel->wp_time_round_item.lock())
        {
            break;
        }
    }
}

void channel::Add_New_Connect(int fd)
{
    SP_channel conn_socket(new channel(fd,readn,writen,time_round));
    stCoRoutine_t *co = 0;
    co_create(&co,NULL,channel::CoroutineFun,&conn_socket);
    co_resume(co);
}

void channel::Close()
{

}

channel::~channel()
{
    close(fd);
    LOG <<"client fd: "<<fd<<" has been closed";
    delete[] read_buff;
    delete[] write_buff;
}

int channel::get_fd()
{
    return fd;
}
void channel::SeparateTimer()
{
    std::shared_ptr<TimeRoundItem<channel>> temp;
    //获得当前fd对应的定时器，如果有的话，需要先分离，lock()是线程安全的
    if(temp = wp_time_round_item.lock())
    {
        temp->reset();//将绑定到该定时器上的fd分离
    }
}

void channel::HandleRead()
{
    read_length = read(shared_from_this(),read_buff,max_read_write_buff_length);
}

void channel::HandleWrite()
{
    int wlen = 0;
    do
    {
        if(wlen != 0)
        {
            write_length -= wlen;
            memcpy(write_buff,write_buff+wlen,write_length);
        }
        wlen = write(shared_from_this(),write_buff,write_length);
    }while(wlen >=0 && wlen < write_length);
    if(wlen < 0)
    {
        write_length = wlen;
    }
}

int channel::handle_close()
{
    //服务器端关闭连接
    std::shared_ptr<TimeRoundItem<channel>> temp;
    if(temp = wp_time_round_item.lock())
    {
        temp->reset();
    }
    Close();
}
