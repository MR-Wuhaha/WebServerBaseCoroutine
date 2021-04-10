#include"LogFile.h"
#include<string.h>
LogFile::LogFile(string filename)
{
    pthread_mutex_init(&put_Mutex,NULL);
    pthread_mutex_init(&get_Mutex,NULL);
    pthread_cond_init(&get_Cond,NULL);
    pthread_cond_init(&Swap_Cond,NULL);
    fd = open(filename.c_str(),O_RDWR | O_APPEND | O_CREAT);
    if(fd < 0)
    {
        cout<<"LofFile open fail"<<endl;
        assert(0);
    }
}

void LogFile::Start_Log()
{
    pthread_t Write_thread;
    pthread_t Swap_thread;
    pthread_create(&Swap_thread,NULL,&LogSwapStart,this);
    pthread_setname_np(Swap_thread,"LogSwapThread");
    cout<<"LogSwapThread Start"<<endl;
    pthread_create(&Write_thread,NULL,&LogWriteStart,this);
    pthread_setname_np(Write_thread,"LogWriteThread");
    cout<<"LogWriteThread Start"<<endl;
}

void LogFile::put_write_string(char* buff)
{
    pthread_mutex_lock(&put_Mutex);
    Wait_WriteTask.push(buff);
    pthread_cond_broadcast(&Swap_Cond);
    pthread_mutex_unlock(&put_Mutex);
}

void LogFile::write_file()
{
    while(true)
    {
        pthread_mutex_lock(&get_Mutex);
        while(WriteTask.size() == 0)
        {
            pthread_cond_broadcast(&Swap_Cond);
            pthread_cond_wait(&get_Cond,&get_Mutex);
        }
        while(WriteTask.size() > 0)
        {
            char* buff = WriteTask.front();
            WriteTask.pop();
            //写日志文件
            write(fd,(void*)buff,strlen(buff));
            //cout<<"Write LogFile Success!"<<endl;
            delete[] buff;
        }
        pthread_mutex_unlock(&get_Mutex);
    }
}

void LogFile::swap()
{
    while(true)
    {
        pthread_mutex_lock(&get_Mutex);
        while(WriteTask.size() != 0)
        {
            pthread_cond_wait(&Swap_Cond,&get_Mutex);
        }
        pthread_mutex_lock(&put_Mutex);
        while(Wait_WriteTask.size() == 0)
        {
            pthread_cond_wait(&Swap_Cond,&put_Mutex);
        }
        WriteTask.swap(Wait_WriteTask);
        pthread_cond_broadcast(&get_Cond);
        pthread_mutex_unlock(&get_Mutex);
        pthread_mutex_unlock(&put_Mutex);
    }
}

void* LogSwapStart(void* ptr)
{
    if(ptr != nullptr)
    {
        ((LogFile*)ptr)->swap();
    }
}

void* LogWriteStart(void* ptr)
{
    if(ptr != nullptr)
    {
        ((LogFile*)ptr)->write_file();
    }
}