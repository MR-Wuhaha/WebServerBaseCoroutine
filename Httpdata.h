#ifndef _HTTPDATA
#define _HTTPDATE
#include"channel.h"
#include<unordered_map>
#include<sys/mman.h>
#include<dirent.h>
struct Http_Handle
{
    string Req;
    string _version;
    string URI;
    unordered_map<string,string> request_head;
    string request_body;
    Http_Handle()
    {
        Req = "";
        _version = "";
        URI = "";
    }
};

class Httpdata:public channel
{
    public:
        Httpdata(int _fd,Handle _read,Handle _write,TimeRound<channel>* time_round);
        void handle_http(char* buff);
        void handle_request_line(char*,int&,Http_Handle*);
        void handle_request_head(char*,int&,Http_Handle*);
        bool Get_KeepAlive_State();
        virtual int handle_event();
        virtual void Add_New_Connect(int);
        ~Httpdata();
    private:
        bool Keep_Alive;
};
#endif