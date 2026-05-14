#pragma once

#include <array>
#include <atomic>
#include <shared_mutex>
#include "Core/Core.hpp"
#include "Core/Logger.hpp"
#include "Core/Types.hpp"

namespace xlux {
    template <typename T>
    class WorkerQueue {
    public:
        WorkerQueue() = default;
        WorkerQueue(WorkerQueue<T>&) = delete;
        WorkerQueue(WorkerQueue<T>&&) = delete;

        void Push(const T& item) {
            std::unique_lock lock(m_Mutex);
            m_Queue.push(item);
        }

        bool Pop(T& item) {
            std::unique_lock lock(m_Mutex);
            if (m_Queue.empty()) {
                return false;
            }
            item = std::move(m_Queue.front());
            m_Queue.pop();
            return true;
        }

        U32 Size() const {
            std::unique_lock lock(m_Mutex);
            return static_cast<U32>(m_Queue.size());
        }

        bool IsEmpty() const {
            std::unique_lock lock(m_Mutex);
            return m_Queue.empty();
        }

    private:
        std::queue<T> m_Queue;
        mutable std::mutex m_Mutex;
    };

    template <typename JobWorker, typename JobPayload>
    concept CJobWorker = requires(JobWorker w, JobPayload p, U32 threadId) {
        { w.Execute(p, threadId) } -> std::same_as<bool>;
    };

    template <typename JobPayload, typename JobWorker>
    requires std::is_copy_constructible_v<JobPayload> && CJobWorker<JobWorker, JobPayload>
    class Worker {
    public:
        Worker(U32 id, RawPtr<JobWorker> jobFunction) : m_ID(id), m_JobFunction(jobFunction) {
            m_Thread = std::thread(&Worker::Run, this);
        }

        ~Worker() {
            m_IsAlive.clear();
            if (m_Thread.joinable()) {
                m_Thread.join();
            }
        }

        Bool IsAlive() const {
            return m_IsAlive.test(std::memory_order::relaxed);
        }

        U32 GetID() const {
            return m_ID;
        }

        U32 GetQueueSize() const {
            return m_JobQueue.Size();
        }

        Bool IsIdle() const {
            return m_JobQueue.IsEmpty();
        }

        void WaitForIdle() const {
            while (!IsIdle()) {
                std::this_thread::yield();
            }
        }

        void AddJob(const JobPayload& job) {
            m_JobQueue.Push(job);
        }

    private:
        void Run() {
            this->m_IsAlive.test_and_set();
            while (this->m_IsAlive.test(std::memory_order::relaxed)) {
                JobPayload job = {};
                if (m_JobQueue.Pop(job)) {
                    if (!m_JobFunction->Execute(job, m_ID)) {
                        // Handle job execution failure if necessary
                    }
                }
            }
        }

    private:
        U32 m_ID = 0;
        RawPtr<JobWorker> m_JobFunction = nullptr;
        WorkerQueue<JobPayload> m_JobQueue;
        std::atomic_flag m_IsAlive = ATOMIC_FLAG_INIT;
        std::thread m_Thread;
    };
    

    template <U32 WorkerCount, typename JobPayload, typename JobWorker>
    class WorkerPool {
    public:

        template <typename... Args>
        WorkerPool(Args&&... args) {
            m_JobFunction = CreateScope<JobWorker>(std::forward<Args>(args)...);
            for (U32 i = 0; i < WorkerCount; ++i) {
                m_Workers[i] = CreateScope<WorkerType>(i, m_JobFunction.get());
            }
        }

        ~WorkerPool() {
            for (auto& worker : m_Workers) {
                worker.reset();
            }
            m_JobFunction.reset();
        }

        void WaitForIdle() const {
            for (const auto& worker : m_Workers) {
                worker->WaitForIdle();
            }
        }

        void AddJob(const JobPayload& job) {
            WorkerType* targetWorker = nullptr;
            U32 minQueueSize = std::numeric_limits<U32>::max();
            for (const auto& worker : m_Workers) {
                U32 queueSize = worker->GetQueueSize();
                if (queueSize < minQueueSize) {
                    minQueueSize = queueSize;
                    targetWorker = worker.get();
                }

                if (queueSize == 0) {
                    break; // Found an idle worker, no need to check further
                }
            }

            if (targetWorker) {
                targetWorker->AddJob(job);
            } else {
                log::Warn("WorkerPool: No available worker to handle the job");
            }
        }

    private:
        using Self = WorkerPool<WorkerCount, JobPayload, JobWorker>;
        using WorkerType = Worker<JobPayload, JobWorker>;

        Array<Scope<WorkerType>, WorkerCount> m_Workers;
        Scope<JobWorker> m_JobFunction;
    };
}