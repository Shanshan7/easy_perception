#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <chrono>
#include <iostream>

namespace Detail
{
    class taskQueue
    {
    public:
        explicit taskQueue(size_t max) : m_max{max} { start(); }
        ~taskQueue() { stop(); }

        bool tryPush(const std::function<void()> &task)
        {
            {
                std::unique_lock<std::mutex> ul(m_mtx);
                if (m_close || m_queue.size() == m_max)
                {
                    return false;
                }
                m_queue.emplace(task);
            }
            m_cv.notify_all();
            return true;
        }

    private:
        taskQueue(const taskQueue &) = delete;

        void start()
        {
            auto th = std::thread([&]() {
                std::cout << "start" << std::endl;
                while (!m_close)
                {
                    std::unique_lock<std::mutex> ul(m_mtx);
                    m_cv.wait(ul, [&]() {
                        return !m_queue.empty() || m_close;
                    });
                    if (m_close)
                    {
                        break;
                    }

                    auto task = m_queue.front();
                    m_queue.pop();
                    ul.unlock();

                    if (task)
                    {
                        task();
                    }
                }

                std::unique_lock<std::mutex> ul(m_mtx);
                while (!m_queue.empty())
                {
                    auto task = m_queue.front();
                    m_queue.pop();
                    if (task)
                    {
                        task();
                    }
                }
                std::cout << "end" << std::endl;
            });
            m_th.swap(th);
        }

        void stop()
        {
            if (m_th.joinable())
            {
                {
                    std::unique_lock<std::mutex> ul(m_mtx);
                    m_close = true;
                }
                m_cv.notify_all();
                m_th.join();
            }
        }

        std::mutex m_mtx;
        std::queue<std::function<void()>> m_queue;
        std::condition_variable m_cv;
        bool m_close{false};
        std::thread m_th;
        size_t m_max{0};
    };
}; // namespace Demo