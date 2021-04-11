#include"channel.h"
#include"Epoll.h"
using namespace std;
channel::channel(int _fd,Handle _read,Handle _write,TimeRound<channel>* _time_round):fd(_fd),write(_write),read(_read),time_round(_time_round),event_loop_thread_pool(nullptr)
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
/*
用于处理数据的协程函数，用于接收数据进行处理，其中的handle_event可以根据需要实现新的虚函数版本
*/
void* channel::CoroutineFun(void* ptr)
{
    co_enable_hook_sys();
    SP_channel channel = *((SP_channel*)(ptr));
    while(true)
    {
        channel->handle_event();
        //客户端主动关闭
        if(channel->read_length < 0 || channel->write_length < 0)
        {
            break;
        }
        //非长连接或者系统开启时间轮并且处于超时的状态
        if(!channel->Get_KeepAlive_State() || (channel->Get_KeepAlive_State() && channel->time_round != nullptr && !channel->wp_time_round_item.lock()))
        {
            break;
        }
    }
}
/*
用于接收分配到的新连接的协程函数
*/
void* channel::HandleNewConnectCoroutineFun(void* ptr)
{
    co_enable_hook_sys();
    SP_channel channel = *((SP_channel*)(ptr));
    uint64_t new_connect_fd;
    while(true)
    {
        channel->HandleRead();
        if(channel->read_length > 0)
        {
            uint64_t* uint64buff = (uint64_t*)(channel->read_buff);
            for(int i = 0;i < channel->read_length;i = i+sizeof(uint64_t))
            {
                new_connect_fd = uint64buff[i/8];
                channel->Add_New_Connect(new_connect_fd);
            }
        }
    }
}

void channel::Add_New_Connect(int fd)
{
    fcntl( fd, F_SETFL, fcntl(fd, F_GETFL,0 ) );//协程需要将当前的fd设置为noblock，而不是由用户自己设置，通过hook的fcntl设置
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
#if LOG_FLAG
    LOG <<"client fd: "<<fd<<" has been closed";
#endif
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

void channel::SetEventLoopThreadPool(EventLoopThreadPool* _event_loop_thread_pool)
{
    this->event_loop_thread_pool = _event_loop_thread_pool;
}

bool channel::Get_KeepAlive_State()
{
    return false;
}
