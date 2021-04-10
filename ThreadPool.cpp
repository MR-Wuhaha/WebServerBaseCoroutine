#include"ThreadPool.h"
#include"LogFile.h"
extern LogFile* Log;

Task* ThreadPool::task_list = NULL;
int ThreadPool::max_thread_num = 0;
int ThreadPool::max_task_num = 0;
vector<pthread_t> ThreadPool::threads;
ThreadPool::ThreadPool(int thread_num,int task_num)
{
    max_thread_num = thread_num;
    max_task_num = task_num;
    threads.resize(thread_num,0);
    task_list = new Task(task_num);
}

void ThreadPool::start_thread()
{
    for(int i = 0;i<max_thread_num;i++)
    {
        pthread_create(&threads[i],NULL,ThreadPool::loop,(void*)(0));
        pthread_setname_np(threads[i],string("TPThread"+i).c_str());
        cout<<"ThreadPool thread:"<<(unsigned int)threads[i]<<" start!"<<endl;
        //pthread_join(threads[i],NULL);
    }
}

void* ThreadPool::loop(void* args)
{
    while(true)
    {
        SP_channel temp = task_list->take_task();
        temp->handle_event();
    }
}

ThreadPool::~ThreadPool()
{
    delete task_list;
}
