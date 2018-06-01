#ifndef MUSH_THREADPOOL
#define MUSH_THREADPOOL

#include <vector>
#include <mutex>
#include <condition_variable>
#include <future>
#include <algorithm>
#include <functional>
#include <atomic>

#include "shared_queue.hpp"
#include "monadic_error.hpp"

namespace mush
{
    class thread_pool
    {
        public:
            template<typename Function, typename... Args>
            using enqueued_future = std::future<typename std::result_of<Function(Args...)>::type>;

            thread_pool(size_t thread_count = std::max(std::thread::hardware_concurrency(), 2u));
           ~thread_pool();

            thread_pool(const thread_pool&) = delete;
            thread_pool& operator=(const thread_pool&) = delete;

            size_t size() const;
            size_t active_threads() const;

            template<typename Function, typename... Args>
            Result<enqueued_future<Function, Args...>> enqueue(Function&& f, Args&&... args);

        private:
            std::vector<std::thread>            threads;
            shared_queue<std::function<void()>> tasks;

            std::mutex                          queue_mutex;
            std::condition_variable             condition;

            std::atomic<bool>                   end = false;
            std::atomic<size_t>                 running = 0;
    };

    inline thread_pool::thread_pool(size_t number_of_threads)
    {
        for (size_t i = 0; i < number_of_threads; ++i)
        {
            threads.emplace_back([&]{
                while(true)
                {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [&]{return end || tasks.empty(); });

                        if (end && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    running++;
                    task();
                    running--;
                }
            });
        }
    }

    inline thread_pool::~thread_pool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            end = true;
        }
        condition.notify_all();

        for(auto& thread : threads)
            thread.join();
    }

    inline size_t thread_pool::size() const
    {
        return threads.size();
    }

    inline size_t thread_pool::active_threads() const
    {
        return running;
    }

    template <typename Function, typename... Args>
    Result<thread_pool::enqueued_future<Function, Args...>> thread_pool::enqueue(Function&& f, Args&&... args)
    {
        using return_type = typename std::result_of<Function(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type>>(
                        std::bind(std::forward<Function>(f), std::forward<Args>(args)...)
                    );

        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (end)
                return Error("submitting to stopped threadpool");

            tasks.push([task](){(*task)();});
        }

        condition.notify_one();

        return result;
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
