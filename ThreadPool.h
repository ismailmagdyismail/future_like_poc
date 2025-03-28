#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <vector>
#include <iostream>

#include "ExecutionContext.hpp"

struct TaskWrapper
{
    std::shared_ptr<ExecutionContext> m_pExecutionContext;
    std::function<void(void)> m_fTask;
};

class ThreadPool
{
public:
    ThreadPool()
    {
        auto threadsCount = std::thread::hardware_concurrency();
        for (int i = 0; i < threadsCount; i++)
        {
            m_oWorkerThreads.push_back(std::thread{&ThreadPool::ConsumeTask, this});
        }
    }

    std::shared_ptr<ExecutionContext> AddTask(std::function<void(void)> p_fTask)
    {
        std::shared_ptr<ExecutionContext> pExecutionContext = std::make_shared<ExecutionContext>();
        TaskWrapper task;
        task.m_fTask = p_fTask;
        task.m_pExecutionContext = pExecutionContext;
        {
            std::lock_guard<std::mutex> lock{m_oTasksMutex};
            m_oTasksQueue.push(task);
        }
        m_oTasksCv.notify_one();
        return pExecutionContext;
    }

    ~ThreadPool()
    {
        std::cout << "Destroying thread pools " << std::endl;
        m_bIsTerminated = true;
        m_oTasksCv.notify_all();
        for (int i = 0; i < m_oWorkerThreads.size(); i++)
        {
            std::cout << "Destroying Worker " << i << std::endl;
            if (m_oWorkerThreads[i].joinable())
            {
                m_oWorkerThreads[i].join();
            }
            std::cout << "Destroyed Worker " << i << std::endl;
        }
    }

private:
    void ConsumeTask()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock{m_oTasksMutex};
            m_oTasksCv.wait(lock, [this]()
                            { return m_bIsTerminated || !m_oTasksQueue.empty(); });
            if (m_bIsTerminated)
            {
                return;
            }
            TaskWrapper taskWrapper = m_oTasksQueue.front();
            m_oTasksQueue.pop();
            taskWrapper.m_fTask();
            taskWrapper.m_pExecutionContext->SetResult(1);
        }
    }

    std::vector<std::thread> m_oWorkerThreads;
    std::queue<TaskWrapper> m_oTasksQueue;
    std::mutex m_oTasksMutex;
    std::condition_variable m_oTasksCv;
    std::atomic<bool> m_bIsTerminated{false};
};