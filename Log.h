#ifndef _LOG
#define _LOG
#include"LogFile.h"
#include<string.h>

class LogMessage
{
    public:
        LogMessage() : max_length(2048)
        {
            message = new char[max_length];
            cur = length = 0;
        }
        LogMessage& operator<<(const char* _message)
        {
            Append(_message,strlen(_message));
            return *this;
        }
        LogMessage& operator<<(int _message)
        {
            const char* buff = to_string(_message).c_str();
            Append(buff,strlen(buff));
            return *this;
        }

        LogMessage& operator<<(const string& _message)
        {
            Append(_message.c_str(),_message.length());
            return *this;
        }

        void Append(const char* buff,int _length)
        {
            if(buff == nullptr || _length + length >= max_length)
            {
                return;
            }
            length += _length;
            for(int i = 0;i<_length;i++)
            {
                message[cur++] = buff[i];
            }
            message[cur] = '\0';
        }

        char* GetMessage()
        {
            char* temp = message;
            message = nullptr;
            return temp;
        }

        ~LogMessage()
        {
            delete[] message;
        }
    private:
        int cur;
        int length;
        char* message;
        const int max_length;
};

class Log
{
    public:
        static LogFile* log_file;

        Log(int line,const char* filename);
        string GetSystemTime();
        LogMessage& PutMessage();
        ~Log();
    private:
        LogMessage log_message;
        int line;
        string filename;
};


#define LOG Log(__LINE__, __FILE__).PutMessage()
#endif
