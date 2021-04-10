#include"Log.h"

LogFile* Log::log_file = nullptr;

Log::Log(int _line,const char* _filename) : line(_line),filename(_filename),log_message()
{
    log_message<<GetSystemTime();
}

string Log::GetSystemTime()
{
    time_t now = time(NULL);
    struct tm* tmstr = localtime(&now);
    char content[512] = { 0 };
    sprintf(content, "[%04d-%02d-%02d %02d:%02d:%02d]",
    tmstr->tm_year + 1900,
    tmstr->tm_mon + 1,
    tmstr->tm_mday,
    tmstr->tm_hour,
    tmstr->tm_min,
    tmstr->tm_sec);
    return string(content);
}

LogMessage& Log::PutMessage()
{
    return log_message;
}

Log::~Log()
{
    log_message<<"     [FileName:"<<filename<<" "<<"Line:"<<line<<"]\n";
    if(log_file != nullptr)
    {
        log_file->put_write_string(log_message.GetMessage());
    }
}