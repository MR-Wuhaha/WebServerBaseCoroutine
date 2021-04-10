#ifndef _TASK
#define _TASK
#include<iostream>
#include<queue>
#include<pthread.h>
#include"channel.h"
using namespace std;
class Task
{
    private:
        pthread_t swap_thread;
        int max_task_num;
        queue<SP_channel> product_task;
        queue<SP_channel> consum_task;
        pthread_mutex_t Product_Mutex;
        pthread_mutex_t Comsumer_Mutex;
        pthread_cond_t Swap_Cond;
        pthread_cond_t ComsumCond;
        pthread_cond_t ProductCond;
    public:
        Task(int);
        SP_channel take_task();
        void put_task(vector<SP_channel>&);
        static void* swap_task(void*);
};
#endif