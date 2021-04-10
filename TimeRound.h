#ifndef TIME_ROUND
#define TIME_ROUND
#include<time.h>
#include<unistd.h>
#include"TimeRoundItem.h"
#include<vector>
#include<pthread.h>
#include<iostream>
#include <functional>
#include<sys/time.h>
using namespace std;
class Lock_Guard
{
    private:
        pthread_mutex_t &Mutex;
    public:
        Lock_Guard(pthread_mutex_t& _Mutex);
        void unlock();
        ~Lock_Guard();
};
class ThreadFun
{
    typedef std::function<void()> Fun;
    public:
        ThreadFun(const Fun& _fun):fun(_fun)
        {

        }
        ThreadFun(){}
        void StartFun()
        {
            fun();
        }
    private:
        Fun fun;
};
template<typename T>
struct SP_TimeRoundItemNode
{
    typedef std::shared_ptr<TimeRoundItem<T>> SP_TimeRoundItem;
    SP_TimeRoundItemNode* pre;
    SP_TimeRoundItemNode* next;
    SP_TimeRoundItem Item;
    SP_TimeRoundItemNode(SP_TimeRoundItem _Item) : Item(_Item),pre(nullptr),next(nullptr){}
};
template<typename T>
struct SP_TimeRoundItemList
{
    SP_TimeRoundItemNode<T>* head;
    SP_TimeRoundItemNode<T>* tail;
    SP_TimeRoundItemList() : head(nullptr),tail(nullptr){}
};

void* start_fun(void* obj);
template<typename T>
class TimeRound
{
    public:
        TimeRound(int _max_time) : max_time(_max_time+1),Thread(0)
        {
            TimeRoundVector.resize(max_time,nullptr);
            time_out_event_list = new SP_TimeRoundItemList<T>();
            cur_index = 0;
            pthread_mutex_init(&Mutex,NULL);
            pthread_cond_init(&Cond,NULL);
            cur_time = 0;//只有线程开始运行时才会计算开始时间
        }

        void AddToTimeOutList(int index)
        {
            if(TimeRoundVector[index] != nullptr && TimeRoundVector[index]->head != nullptr)
            {
                if(time_out_event_list->head == nullptr)
                {
                    time_out_event_list->head = TimeRoundVector[index]->head;
                    time_out_event_list->tail = TimeRoundVector[index]->tail;
                }
                else
                {
                    time_out_event_list->tail->next = TimeRoundVector[index]->head;
                    time_out_event_list->tail = TimeRoundVector[index]->tail;
                }
                TimeRoundVector[index]->head = TimeRoundVector[index]->tail = nullptr;
            }
        }

        void UpdateTimeRoundItemNode(SP_TimeRoundItemNode<T>* time_round_item_node,int time)
        {
            int index = time_round_item_node->Item->GetIndex();
            if(time >= max_time)
            {
                cout<<"UpdateTime time is larger than the max_time of TimeRound,Set to the max_time-1"<<endl;
                time = max_time-1;
            }
            int new_index = (cur_index + time)%max_time;
            if(new_index == index)
                return;
            Lock_Guard lock(Mutex);
            time_round_item_node->Item->SetIndex(new_index);
            if(time_round_item_node == TimeRoundVector[index]->head)
            {
                if(time_round_item_node == TimeRoundVector[index]->tail)
                {
                    TimeRoundVector[index]->head = TimeRoundVector[index]->tail = nullptr;
                }
                else
                {
                    TimeRoundVector[index]->head = time_round_item_node->next;
                    TimeRoundVector[index]->head->pre = nullptr;
                }
            }
            else if(time_round_item_node == TimeRoundVector[index]->tail)
            {
                TimeRoundVector[index]->tail = time_round_item_node->pre;
                TimeRoundVector[index]->tail->next = nullptr;
            }
            else
            {
                time_round_item_node->pre->next = time_round_item_node->next;
                time_round_item_node->next->pre = time_round_item_node->pre;
            }
            if(TimeRoundVector[new_index]->head == nullptr)
            {
                TimeRoundVector[new_index]->head = TimeRoundVector[new_index]->tail = time_round_item_node;
                time_round_item_node->pre = nullptr;
                time_round_item_node->next = nullptr;
            }
            else
            {
                TimeRoundVector[new_index]->tail->next = time_round_item_node;
                time_round_item_node->pre = TimeRoundVector[new_index]->tail;
                TimeRoundVector[new_index]->tail = time_round_item_node;
                time_round_item_node->next = nullptr;
            }
        }

        void start()
        {
            gettimeofday(&system_time,NULL);
            cur_time = system_time.tv_sec;
            thread_fun = ThreadFun(bind(&TimeRound::fun,this));
            pthread_create(&Thread,NULL,&start_fun,&thread_fun);
            pthread_setname_np(Thread,"TRThread");
            cout<<"TimeRoundThread Start"<<endl;
        }
        void fun()
        {
            while(true)
            {
                gettimeofday(&system_time,NULL);
                time_t diff_time = system_time.tv_sec - cur_time;
                if(diff_time >= 1)
                {
                    if(diff_time >= max_time)
                    {
                        diff_time = max_time - 1;
                    }
                    cur_time = system_time.tv_sec;
                    Lock_Guard lock(Mutex);
                    while(diff_time--)
                    {
                        AddToTimeOutList(cur_index);
                        cur_index++;
                        if(cur_index >= max_time)
                        {
                            cur_index = 0;
                        }
                    }
                    lock.unlock();
                    HandleTimeOutList();
                }
                sleep(1);
            }
        }

        void AddTimeRoundItemNode(SP_TimeRoundItemNode<T>* time_round_item_node)
        {
            Lock_Guard lock(Mutex);
            if(time_round_item_node == nullptr)
            {
                return;
            }
            int time = time_round_item_node->Item->GetTimeThreod();
            if(time <= 0)
            {
                delete time_round_item_node;
                return;
            }
            if(time >= max_time)
            {
                cout<<"TimeOut time is larger than the max_time of TimeRound,Set to the max_time-1"<<endl;
                time = max_time-1;
            }
            int index = (cur_index + time) % max_time;
            time_round_item_node->Item->SetIndex(index);

            if(TimeRoundVector[index] == nullptr)
            {
                TimeRoundVector[index] = new SP_TimeRoundItemList<T>();
                TimeRoundVector[index]->head = time_round_item_node;
                TimeRoundVector[index]->tail = time_round_item_node;
            }
            else
            {
                if(TimeRoundVector[index]->head != nullptr)
                {
                    TimeRoundVector[index]->tail->next = time_round_item_node;
                    time_round_item_node->pre = TimeRoundVector[index]->tail;
                    TimeRoundVector[index]->tail = time_round_item_node;
                }
                else
                {
                    TimeRoundVector[index]->head = time_round_item_node;
                    TimeRoundVector[index]->tail = time_round_item_node;
                }
            }
        }

        void HandleTimeOutList()
        {
            if(time_out_event_list != nullptr)
            {
                SP_TimeRoundItemNode<T>* head = time_out_event_list->head;
                time_out_event_list->head = nullptr;
                time_out_event_list->tail = nullptr;
                while(head != nullptr)
                {
                    SP_TimeRoundItemNode<T>* temp_head = head;
                    head = head->next;
                    delete temp_head;
                }
            }
        }
        ~TimeRound()
        {
            HandleTimeOutList();
            delete time_out_event_list;
        }
    private: 
        int max_time;//时间轮能够允许的最长运行时间
        SP_TimeRoundItemList<T>* time_out_event_list;//超时待处理队列
        vector<SP_TimeRoundItemList<T>*> TimeRoundVector;
        time_t cur_time;//时间轮当前的时间
        int cur_index;//时间论运行的当前位置
        pthread_t Thread;//运行时间轮的线程
        pthread_mutex_t Mutex;//操作时间轮队列必须加锁
        pthread_cond_t Cond;
        ThreadFun thread_fun;

        timeval system_time;
        tm tm_time;
};
#endif