#ifndef MUSH_SHARED_QUEUE
#define MUSH_SHARED_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

namespace mush
{
    template <typename T>
    class shared_queue
    {
        public:
            shared_queue() = default;
           ~shared_queue() = default;

            T& front();
            void pop();

            void push(const T& item);
            void push(T item);

            int size();
            bool empty();

        private:
            std::queue<T>           queue;
            std::mutex              queue_lock;
            std::condition_variable condition;
    };

    template <typename T>
    T& shared_queue<T>::front()
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        while (queue.empty())
            condition.wait(lock);

        return queue.front();
    }

    template <typename T>
    void shared_queue<T>::pop()
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        while(queue.empty())
            condition.wait(lock);

        queue.pop();
    }

    template <typename T>
    void shared_queue<T>::push(const T& item)
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        queue.push(item);
        lock.unlock();
        condition.notify_one();
    }

    template <typename T>
    void shared_queue<T>::push(T item)
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        queue.push(std::move(item));
        lock.unlock();
        condition.notify_one();
    }

    template <typename T>
    int shared_queue<T>::size()
    {
        std::unique_lock<std::mutex> lock(queue_lock);
        return queue.size();
    }
}

#endif
