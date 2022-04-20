#include "ThreadPool/ThreadPool.h"

namespace tp 
{

	ThreadPool::ThreadPool(unsigned int threadCount)
		: m_threadCount(threadCount)
		, m_jobsLeft{0}
		, m_bailout{false}
		, m_finished{false}
	{
		for (unsigned int i = 0; i < m_threadCount; ++i)
		{
			m_threads.push_back(std::thread([this, i]
			{
				this->Task();
			}));
		}
	}

	ThreadPool::~ThreadPool()
	{
		JoinAll();
	}

	uint32_t ThreadPool::Size() const
	{
		return m_threadCount;
	}

	size_t ThreadPool::JobsRemaining()
	{
		std::lock_guard<std::mutex> guard(m_queueMutex);
		return m_queue.size();
	}

	void ThreadPool::AddJob(std::function<void(void)> job)
	{
		std::lock_guard<std::mutex> guard(m_queueMutex);
		m_queue.emplace_back(job);
		++m_jobsLeft;
		m_jobAvailableVar.notify_one();
	}

	
	void ThreadPool::JoinAll(bool WaitForAll)
	{
		if (!m_finished)
		{
			if (WaitForAll)
			{
				WaitAll();
			}

			// note that we're done, and wake up any thread that's
			// waiting for a new job
			m_bailout = true;
			m_jobAvailableVar.notify_all();

			for (auto &x : m_threads)
			{
				if (x.joinable())
				{
					x.join();
				}
			}
			m_finished = true;
		}
	}

	
	void ThreadPool::WaitAll()
	{
		if (m_jobsLeft > 0)
		{
			std::unique_lock<std::mutex> lk(m_waitMutex);
			
			m_waitVar.wait(lk, [this]
			{
				return this->m_jobsLeft == 0;
			});
			
			lk.unlock();
		}
	}


	
	void ThreadPool::Task()
	{
		while (!m_bailout)
		{
			NextJob()();
			--m_jobsLeft;
			m_waitVar.notify_one();
		}
	}

	
	std::function<void(void)> ThreadPool::NextJob()
	{
		std::function<void(void)> res;
		std::unique_lock<std::mutex> job_lock(m_queueMutex);

		// Wait for a job if we don't have any.
		m_jobAvailableVar.wait(job_lock, [this]
		{
			return m_queue.size() || m_bailout;
		});

		// Get job from the queue
		if (!m_bailout)
		{
			res = m_queue.front();
			m_queue.pop_front();
		}
		else
		{
			// If we're bailing out, 'inject' a job into the queue to keep jobs_left accurate.
			res = []{};
			++m_jobsLeft;
		}
		return res;
	}


} // namespace tp
