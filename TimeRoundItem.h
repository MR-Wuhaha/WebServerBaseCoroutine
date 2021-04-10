#ifndef TIME_ROUND_ITEM
#define TIME_ROUND_ITEM
#include<functional>
#include<memory>
typedef std::function<void()> Fun;
template<typename T>
class TimeRoundItem
{
    public:
        TimeRoundItem(int _time_threod,Fun _fun,std::shared_ptr<T> _ptr):
        time_threod(_time_threod),fun(_fun),ptr(_ptr)
        {
            index = 0;
        }

        ~TimeRoundItem()
        {
            if(ptr && fun)
            {
                fun();
            }
        }
        int GetIndex()
        {
            return index;
        }

        void SetIndex(int new_index)
        {
            index = new_index;
        }

        int GetTimeThreod()
        {
            return time_threod;
        }
        void reset()
        {
            if(ptr)
            {
                ptr.reset();
            }
        }
    private:
        int index;//在时间轮中的位置
        int time_threod;//超时时间阈值
        Fun fun;//时间到时执行的函数
        std::shared_ptr<T> ptr;//管理对象的指针
};
#endif