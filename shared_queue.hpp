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
/*
 Copyright (c) 2018 Jari Ronkainen

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose, including
    commercial applications, and to alter it and redistribute it freely, subject to
    the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim
       that you wrote the original software. If you use this software in a product,
       an acknowledgment in the product documentation would be appreciated but is
       not required.

    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/
