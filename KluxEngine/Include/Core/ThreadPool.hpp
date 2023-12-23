#pragma once

#include "Core/Core.hpp"

namespace klux
{
	template <typename JobPayload, typename JobResult>
	class IJob
	{
	public:
		virtual ~IJob() = default;
		virtual Bool Execute(const JobPayload& payload, JobResult& result) = 0;
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
			while (HasJob() && IsAlive())
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
		PoolWorker(RawPtr<IJob<JobPayload, JobResult>> job)
		{
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
					std::this_thread::sleep_for(std::chrono::milliseconds(m_SleepDuration));
				}

				if (m_Jobs.size() > 0)
				{

					{
						std::lock_guard<std::mutex> lock(m_JobMutex);
						job = m_Jobs.front();
						m_Jobs.pop();
					}

					Bool res = m_Job->Execute(job, result);

					if (res)
					{
						std::lock_guard<std::mutex> lock(m_JobDoneMutex);
						m_JobDone.push(result);
					}
				}
			}
			m_IsRunning = false;
		}

	private:
		std::mutex m_JobMutex;
		std::mutex m_JobDoneMutex;
		RawPtr<IJob<JobPayload, JobResult>> m_Job;
		Queue<JobPayload> m_Jobs;
		Queue<JobResult> m_JobDone;
		Bool m_IsAlive = false;
		Bool m_IsPaused = false;
		Bool m_IsRunning = false;
		std::thread m_Thread;
		Size m_SleepDuration = 2;
	};

	namespace internal_
	{
		template <typename T>
		concept IdCon = std::is_integral_v<T>;
	}

	template<U32 JobCount, typename JobPayload, typename JobResult>
	class ThreadPool
	{
	public:
		ThreadPool(RawPtr<IJob<JobPayload, JobResult>> job)
		{
			for (U32 i = 0; i < JobCount; ++i)
				m_Workers[i] = new PoolWorker<JobPayload, JobResult>(job);
		}

		~ThreadPool()
		{
			for (U32 i = 0; i < JobCount; ++i)
				delete m_Workers[i];
		}


		template <internal_::IdCon... Ids>
		inline void Pause()
		{
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
					m_Workers[i]->Pause();
			}
			else
			{
				(m_Workers[Ids]->Pause(), ...);
			}
		}

		template <internal_::IdCon... Ids>
		inline void Resume()
		{
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
					m_Workers[i]->Resume();
			}
			else
			{
				(m_Workers[Ids]->Resume(), ...);
			}
		}

		template <internal_::IdCon... Ids>
		inline void WaitJobDone()
		{
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
					m_Workers[i]->WaitJobDone();
			}
			else
			{
				(m_Workers[Ids]->WaitJobDone(), ...);
			}
		}

		inline U32 GetWorkerCount() const
		{
			return JobCount;
		}

		template <internal_::IdCon... Ids>
		inline Bool HasJob() const
		{
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
				{
					if (m_Workers[i]->HasJob())
						return true;
				}
				return false;
			}
			else
			{
				return (m_Workers[Ids]->HasJob() || ...);
			}
		}

		template <internal_::IdCon... Ids>
		inline Bool HasResult() const
		{
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
				{
					if (m_Workers[i]->HasResult())
						return true;
				}
				return false;
			}
			else
			{
				return (m_Workers[Ids]->HasResult() || ...);
			}
		}

		inline U32 AddJob(const JobPayload& job)
		{
			std::lock_guard<std::mutex> lock(m_JobMutex);
			U32 id = m_CurrentWorker;
			m_Workers[m_CurrentWorker]->AddJob(job);
			m_CurrentWorker = (m_CurrentWorker + 1) % JobCount;
			return id;
		}

		template <internal_::IdCon... Ids>
		inline std::array<JobResult, sizeof...(Ids)> GetJobResult()
		{
			std::array<JobResult, sizeof...(Ids)> results;
			if constexpr (sizeof...(Ids) == 0)
			{
				for (U32 i = 0; i < JobCount; ++i)
				{
					if (m_Workers[i]->HasResult())
						results[i] = m_Workers[i]->GetJobResult();
				}
			}
			else
			{
				U32 i = 0;
				((m_Workers[Ids]->HasResult() ? results[i++] = m_Workers[Ids]->GetJobResult() : 0), ...);
			}
			return results;
		}

	private:
		RawPtr<PoolWorker<JobPayload, JobResult>> m_Workers[JobCount];
		U32 m_CurrentWorker = 0;
		std::mutex m_JobMutex;
	};

}
