#include"Httpdata.h"
#include"Epoll.h"
#include"Log.h"
Httpdata::Httpdata(int _fd,Handle _read,Handle _write,TimeRound<channel>* _time_round):channel(_fd,_read,_write,_time_round),
Keep_Alive(false)
{

}

int Httpdata::handle_event()
{
    HandleRead();
    if(read == readn)
    {
        //cout<<"recv data from client: "<<fd<<" "<<buff<<endl;
        //有读取到数据才需要处理
        if(read_length > 0)
        {
            handle_http(read_buff);
            SeparateTimer();
            if(time_round != nullptr && Keep_Alive)
            {
                std::shared_ptr<TimeRoundItem<channel>> sp_time_round_item(new TimeRoundItem<channel>(5,bind(&Httpdata::Close,shared_from_this().get()),shared_from_this()));
                wp_time_round_item = std::weak_ptr<TimeRoundItem<channel>>(sp_time_round_item);
                SP_TimeRoundItemNode<channel>* sp_time_round_item_node = new SP_TimeRoundItemNode<channel>(sp_time_round_item);
                time_round->AddTimeRoundItemNode(sp_time_round_item_node);
            }
        }
    }
    return 0;
}

void Httpdata::Add_New_Connect(int fd)
{
    fcntl( fd, F_SETFL, fcntl(fd, F_GETFL,0 ) );//协程需要将当前的fd设置为noblock，而不是由用户自己设置，通过hook的fcntl设置
    SP_channel conn_socket(new Httpdata(fd,readn,writen,time_round));
    conn_socket->SetEventLoop(this->event_loop);
    stCoRoutine_t* co = 0;
    co_create(&co,NULL,channel::CoroutineFun,&conn_socket);
    co_resume(co);
}

void Httpdata::handle_http(char *buff)
{
    int index = 0;
    Http_Handle *Recv_Http = new Http_Handle();
    handle_request_line(buff,index,Recv_Http);
    handle_request_head(buff,index,Recv_Http);
    if(Recv_Http->request_head.count("Connection") != 0 &&  
        (Recv_Http->request_head["Connection"] == "keep-alive" || 
        Recv_Http->request_head["Connection"] == "Keep-Alive") )
    {
        Keep_Alive = true;
    }
    string head = "";
    string body = "";
    //if(Recv_Http->URI == "222")
    if(Recv_Http->URI == "/")
    {
        body += "<html><title>Source Page</title>";
        body += "<body bgcolor=\"ffffff\">";
        body += "!Thank for visiting MRWu_haha's Server!";
        
        body += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";
        body += "<br><a href=\"http://www.baidu.com\">百度一下</a>";
        body += "<br><a href=\"https://join.qq.com\">腾讯招聘</a>";
        body += "<br><a href=\"https://campus.kuaishou.cn\">快手招聘</a>";
        body += "<br><a href=\"https://uniportal.huawei.com\">华为招聘</a>";
        body += "<br><a href=\"https://campus.163.com\">网易招聘</a>";
        body += "<br><a href=\"https://careers.pinduoduo.com\">拼多多招聘</a>";
        body += "<br><a href=\"https://campus.alibaba.com\">阿里巴巴招聘</a>";
        body += "<br><a href=\"https://campus.bigo.sg\">BIGO招聘</a><br>";
        body += "<font size=\"5\"> MR-Wuhaha's Source Flie List: </font><br/>";
        DIR* source = opendir("/source");
        if(source != nullptr)
        {
            dirent* p_entry = nullptr;
            while((p_entry = readdir(source)) != nullptr)
            {
                if(!(p_entry->d_type & S_IFDIR) && (strcmp(".", p_entry->d_name) != 0 && strcmp("..", p_entry->d_name) != 0))
                {
                    body += "<br><a href=\"/source/" + string(p_entry->d_name) + "\">" + string(p_entry->d_name) + "</a>";
                }
            }
            closedir(source);
        }
        
        body += "</body></html>";
        head += Recv_Http->_version + " 200 OK\r\n";
        head += "Connection: Keep-Alive\r\n";
        head += "Content-type: text/html\r\n";
        head += "Content-Length: " + to_string(body.size()) + "\r\n";
        head += "\r\n";
    }
    else if(Recv_Http->URI == "/hello")
    {
        body += "<html><title>Hello</title>";
        body += "<body bgcolor=\"ffffff\">";
        body += "Thank for visiting this server!!!!";
        body += "<hr><em> MR_Wuhaha's WebServer </em>\n</body></html>";
        head += Recv_Http->_version + " 200 OK\r\n";
        head += "Connection: Keep-Alive\r\n";
        head += "Content-Type: text/html\r\n";
        head += "Content-Length: " + to_string(body.size()) + "\r\n";
        head += "\r\n";
    }
    else
    {
        string request_file_name = Recv_Http->URI;
        int request_fd = open(request_file_name.c_str(),O_RDONLY);
        if(request_fd < 0)
        {
            cout<<request_file_name<<" : openfail"<<endl;
        }
        char* ptr = nullptr;
        if(request_fd > 0 && (ptr = (char*)mmap(NULL,4096,PROT_READ,MAP_SHARED,request_fd,0)) != MAP_FAILED)
        {
            body += "<html>";
            body += "<title>" + Recv_Http->URI.substr(1) +  "</title>";
            body += "<head>";
            body += "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";
            body += "</head>";
            body += "<body bgcolor=\"ffffff\">";
            body += "MR_Wuhaha's WebServer";
            body += "<hr><pre style=\"word-wrap: break-word; white-space: pre-wrap; white-space: -moz-pre-wrap\">";
            body += string(ptr);
            body += "</pre>\n</body></html>";
            head += Recv_Http->_version + " 200 OK\r\n";
            head += "Connection: Keep-Alive\r\n";
            head += "Content-Type: text/html\r\n";
            head += "Content-Length: " + to_string(body.size()) + "\r\n";
            head += "\r\n";
            assert(munmap((void*)ptr,4096) == 0);
            close(request_fd);
        }
        else
        {
            body += "<html><title>Source Not Found</title>";
            body += "<body bgcolor=\"ffffff\">";
            body += "404 NOT_FOUND";
            body += "<hr><em> MR_Wuhaha's WebServer </em>\n</body></html>";
            head += Recv_Http->_version + " 404 NOTFOUND\r\n";
            head += "Connection: Keep-Alive\r\n";
            head += "Content-Type: text/html\r\n";
            head += "Content-Length: " + to_string(body.size()) + "\r\n";
            head += "\r\n";
        }
    }
    delete Recv_Http;
    string send_str = head + body;
    sprintf(write_buff, "%s", send_str.c_str());
    write_length = send_str.length();
    HandleWrite();
}

void Httpdata::handle_request_line(char* buff,int& index,Http_Handle* Recv_Http)
{
    string temp = "";
    while(index < read_length && buff[index] != ' ')
    {
        temp += buff[index];
        index++;
    }
    Recv_Http->Req = temp;
    index++;
    temp  = "";
    while(index < read_length && buff[index] != ' ')
    {
        temp += buff[index];
        index++;
    }
    Recv_Http->URI = temp;
    index++;
    temp = "";
    while(index < read_length && buff[index] != '\r' && buff[index] != '\n')
    {
        temp += buff[index];
        index++;
    }
    Recv_Http->_version = temp;
    index = index+2;
#if LOG_FLAG
    LOG<<Recv_Http->Req+" "+Recv_Http->URI+" "+Recv_Http->_version;
#endif
    //cout<<Recv_Http->Req<<" "<<Recv_Http->URI<<" "<<Recv_Http->_version<<endl;
}

void Httpdata::handle_request_head(char* buff,int &index,Http_Handle* Recv_Http)
{
    while(index < read_length && buff[index] != '\r' && buff[index] != '\n')
    {
        string key = "";
        string value = "";
        while(index < read_length && buff[index] != ':')
        {
            key += buff[index];
            index++;
        }
        index = index+2;
        while(index < read_length && buff[index] != '\r' && buff[index] != '\n')
        {
            value += buff[index];
            index++;
        }
        Recv_Http->request_head[key] = value;
        index = index+2;
    }
    index++;
}

bool Httpdata::Get_KeepAlive_State()
{
    return Keep_Alive;
}

Httpdata::~Httpdata()
{

}