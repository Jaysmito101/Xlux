#pragma once

#include "Core/Core.hpp"

namespace xlux
{
	template <typename JobPayload, typename JobResult>
	class PoolWorker;

	template <typename JobPayload, typename JobResult>
	class IJob
	{
	public:
		virtual ~IJob() = default;
		virtual Bool Execute(JobPayload payload, JobResult& result, Size m_ThreadID) = 0;
	};

	template <typename JobPayload, typename JobResult>
	class PoolWorker
	{
	public:

		inline void Pause()
		{
			m_IsPaused = true;
		}

		inline void Resume()
		{
			m_IsPaused = false;
		}


		inline void AddJob(const JobPayload& job)
		{
			std::lock_guard<std::mutex> lock(m_JobMutex);
			m_Jobs.push(job);
		}

		inline Bool HasJob() const
		{
			return !m_Jobs.empty();
		}

		inline void WaitJobDone()
		{
			while ((HasJob() && IsAlive()) || IsWorking())
			{
				std::this_thread::yield();
			}
		}

		inline Bool HasResult() const
		{
			return !m_JobDone.IsEmpty();
		}

		inline JobResult GetJobResult()
		{
			std::lock_guard<std::mutex> lock(m_JobDoneMutex);
			JobResult result = m_JobDone.front();
			m_JobDone.pop();
			return result;
		}

		inline Bool IsWorking() const
		{
			return m_IsWorking;
		}

		inline Bool IsAlive() const
		{
			return m_IsAlive;
		}

		inline Bool IsPaused() const
		{
			return m_IsPaused;
		}

		inline void SetSleepDuration(Size duration)
		{
			m_SleepDuration = duration;
		}

		template <U32 P, typename Q, typename R>
		friend class ThreadPool;

	private:
		PoolWorker(RawPtr<IJob<JobPayload, JobResult>> job, Size id)
		{
			m_ID = id;	
			m_Job = job;
			m_IsAlive = true;
			m_Thread = std::thread(&PoolWorker::Run, this);
		}

		~PoolWorker()
		{
			m_IsAlive = false;

			if (m_Thread.joinable())
				m_Thread.join();
		}

		void Run()
		{
			JobPayload job;
			JobResult result;
			m_IsRunning = true;
			while (m_IsAlive)
			{
				while ((m_IsPaused || m_Jobs.empty()) && m_IsAlive)
				{
					std::this_thread::sleep_for(std::chrono::microseconds(m_SleepDuration));
					 //std::this_thread::yield();
				}

				if (m_Jobs.size() > 0)
				{
					m_IsWorking = true;

					{
						std::lock_guard<std::mutex> lock(m_JobMutex);
						job = m_Jobs.front();
						m_Jobs.pop();
					}

					Bool res = m_Job->Execute(job, result, m_ID);

					if (res)
					{
						std::lock_guard<std::mutex> lock(m_JobDoneMutex);
						m_JobDone.push(result);
					}

					m_IsWorking = false;
				}
			}
			m_IsRunning = false;
		}

	private:
		Size m_ID = 0;
		std::mutex m_JobMutex;
		std::mutex m_JobDoneMutex;
		RawPtr<IJob<JobPayload, JobResult>> m_Job;
		Queue<JobPayload> m_Jobs;
		Queue<JobResult> m_JobDone;
		Bool m_IsAlive = false;
		Bool m_IsPaused = false;
		Bool m_IsRunning = false;
		Bool m_IsWorking = false;
		std::thread m_Thread;
		Size m_SleepDuration = 10;
	};

	template<U32 JobCount, typename JobPayload, typename JobResult>
	class ThreadPool
	{
	public:
		ThreadPool(RawPtr<IJob<JobPayload, JobResult>> job)
		{
			for (U32 i = 0; i < JobCount; ++i) {
				m_Workers[i] = new PoolWorker<JobPayload, JobResult>(job, i);
			}
		}

		~ThreadPool()
		{
			for (U32 i = 0; i < JobCount; ++i)
				delete m_Workers[i];
		}


		inline void WaitJobDone()
		{
				for (U32 i = 0; i < JobCount; ++i)
					m_Workers[i]->WaitJobDone();
		}

		inline U32 GetWorkerCount() const
		{
			return JobCount;
		}

		inline Bool HasJob() const
		{
			for (U32 i = 0; i < JobCount; ++i)
			{
				if (m_Workers[i]->HasJob()) {
					return true;
				}
			}
			return false;
		}

		inline U32 AddJob(const JobPayload& job)
		{
			std::lock_guard<std::mutex> lock(m_JobMutex);
			U32 id = (m_CurrentWorker = (m_CurrentWorker + 1) % JobCount);
			m_Workers[id]->AddJob(job);
			return id;
		}

		inline U32 AddJobTo(const JobPayload& job, U32 threadID)
		{
			// std::lock_guard<std::mutex> lock(m_JobMutex); // not needed
			U32 id = threadID;
			m_Workers[id]->AddJob(job);
			return id;
		}

	private:
		RawPtr<PoolWorker<JobPayload, JobResult>> m_Workers[JobCount];
		U32 m_CurrentWorker = 0;
		std::mutex m_JobMutex;
	};

}
