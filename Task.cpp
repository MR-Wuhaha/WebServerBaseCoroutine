#include"Task.h"
Task::Task(int _max_task_size):
max_task_num(_max_task_size)
{
    pthread_mutex_init(&Product_Mutex,NULL);
    pthread_mutex_init(&Comsumer_Mutex,NULL);
    pthread_cond_init(&Swap_Cond,NULL);
    pthread_cond_init(&ComsumCond,NULL);
    pthread_cond_init(&ProductCond,NULL);
    pthread_create(&swap_thread,NULL,Task::swap_task,(void*)(this));
}

SP_channel Task::take_task()
{
    pthread_mutex_lock(&Comsumer_Mutex);
    while(consum_task.size() == 0)
    {
        pthread_cond_signal(&Swap_Cond);
        pthread_cond_wait(&ComsumCond,&Comsumer_Mutex);
    }
    SP_channel temp = consum_task.front();
    consum_task.pop();
    pthread_mutex_unlock(&Comsumer_Mutex);
    return temp;
}

void Task::put_task(vector<SP_channel>& new_task)
{
    pthread_mutex_lock(&Product_Mutex);
    for(int i = 0;i<new_task.size();i++)
    {
        while(product_task.size() == max_task_num)
        {
            pthread_cond_signal(&Swap_Cond);
            pthread_cond_wait(&ProductCond,&Product_Mutex);
        }
        product_task.push(new_task[i]);
    }
    pthread_cond_signal(&Swap_Cond);
    pthread_mutex_unlock(&Product_Mutex);
}

void* Task::swap_task(void* args)
{
    Task* obj = (Task*)(args);
    while(true)
    {
        pthread_mutex_lock(&obj->Comsumer_Mutex);
        while(obj->consum_task.size() != 0)
        {
            pthread_cond_wait(&obj->Swap_Cond,&obj->Comsumer_Mutex);
        }
        pthread_mutex_lock(&obj->Product_Mutex);
        while(obj->product_task.size() == 0)
        {
            pthread_cond_wait(&obj->Swap_Cond,&obj->Product_Mutex);
        }
        obj->product_task.swap(obj->consum_task);
        pthread_cond_broadcast(&obj->ComsumCond);
        pthread_cond_broadcast(&obj->ProductCond);
        pthread_mutex_unlock(&obj->Product_Mutex);
        pthread_mutex_unlock(&obj->Comsumer_Mutex);
    }
}