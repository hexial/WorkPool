/*
 * This file is part of WorkPool.
 *
 * Copyright (C) 2024 Magnus Strömbrink
 *
 * WorkPool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WorkPool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with WorkPool. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <stack>
#include <thread>

namespace WorkPool
{

template <typename T>
class Pool
{

  public:
    Pool<T>(size_t size)
    {
        for (size_t i = 0; i < size; i++)
        {
            this->workers.emplace_back(&Pool<T>::Worker, this);
        }
    }

    ~Pool<T>()
    {
        this->stop = true;
        for (auto &worker : this->workers)
        {
            worker.join();
        }
    }

    template <class F, class... Args>
    auto Execute(F &&f, Args &&...args) -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        using return_type = typename std::invoke_result_t<F, Args...>;
        auto job = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = job->get_future();
        {
            std::lock_guard<std::mutex> lock(this->mutex);
            if (this->stop)
                throw std::runtime_error("Execute on stopped Pool");

            this->jobs.emplace([job]() { (*job)(); });
        }
        return res;
    }

    bool Idle()
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        return this->jobs.empty() && this->active == 0;
    }

  protected:
    void Worker()
    {
        while (!stop)
        {
            this->mutex.lock();
            if (this->jobs.empty())
            {
                this->mutex.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            else
            {
                auto job = std::move(this->jobs.top());
                this->jobs.pop();
                this->active++;
                this->mutex.unlock();
                job();
                this->mutex.lock();
                this->active--;
                this->mutex.unlock();
            }
        }
    }

    size_t active = 0;
    std::mutex mutex;
    bool stop = false;
    std::stack<std::function<void()>> jobs;
    std::vector<std::thread> workers;
};

} // namespace WorkPool
