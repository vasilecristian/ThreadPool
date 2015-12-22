#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <array>
#include <list>
#include <functional>
#include <condition_variable>

namespace tp 
{
	/**
	*  Simple ThreadPool that creates `ThreadCount` threads upon its creation,
	*  and pulls from a queue to get new jobs. The default is 10 threads.
	*
	*  This class requires a number of c++11 features be present in your compiler.
	*/
	template <unsigned int ThreadCount = 10>
	class ThreadPool 
	{
		std::array<std::thread, ThreadCount> m_threads;
		std::list<std::function<void(void)> > m_queue;

		std::atomic_int         m_jobsLeft;
		std::atomic_bool        m_bailout;
		std::atomic_bool        m_finished;
		std::condition_variable m_jobAvailableVar;
		std::condition_variable m_waitVar;
		std::mutex              m_waitMutex;
		std::mutex              m_queueMutex;

		/**
		*  Take the next job in the queue and run it.
		*  Notify the main thread that a job has completed.
		*/
		void Task() 
		{
			while (!m_bailout)
			{
				NextJob()();
				--m_jobsLeft;
				m_waitVar.notify_one();
			}
		}

		/**
		*  Get the next job; pop the first item in the queue,
		*  otherwise wait for a signal from the main thread.
		*/
		std::function<void(void)> NextJob() 
		{
			std::function<void(void)> res;
			std::unique_lock<std::mutex> job_lock(m_queueMutex);

			// Wait for a job if we don't have any.
			m_jobAvailableVar.wait(job_lock, [this]{ return m_queue.size() || m_bailout; });

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

	public:
		ThreadPool()
			: m_jobsLeft(ATOMIC_VAR_INIT(0))
			, m_bailout(ATOMIC_VAR_INIT(false))
			, m_finished(ATOMIC_VAR_INIT(false))
		{
			for (unsigned i = 0; i < ThreadCount; ++i)
				m_threads[i] = std::move(std::thread([this, i]{ this->Task(); }));
		}

		/**
		*  JoinAll on deconstruction
		*/
		~ThreadPool() 
		{
			JoinAll();
		}

		/**
		*  Get the number of threads in this pool
		*/
		inline unsigned Size() const 
		{
			return ThreadCount;
		}

		/**
		*  Get the number of jobs left in the queue.
		*/
		inline unsigned JobsRemaining() 
		{
			std::lock_guard<std::mutex> guard(queue_mutex);
			return queue.size();
		}

		/**
		*  Add a new job to the pool. If there are no jobs in the queue,
		*  a thread is woken up to take the job. If all threads are busy,
		*  the job is added to the end of the queue.
		*/
		void AddJob(std::function<void(void)> job) 
		{
			std::lock_guard<std::mutex> guard(m_queueMutex);
			m_queue.emplace_back(job);
			++m_jobsLeft;
			m_jobAvailableVar.notify_one();
		}

		/**
		*  Join with all threads. Block until all threads have completed.
		*  Params: WaitForAll: If true, will wait for the queue to empty
		*          before joining with threads. If false, will complete
		*          current jobs, then inform the threads to exit.
		*  The queue will be empty after this call, and the threads will
		*  be done. After invoking `ThreadPool::JoinAll`, the pool can no
		*  longer be used. If you need the pool to exist past completion
		*  of jobs, look to use `ThreadPool::WaitAll`.
		*/
		void JoinAll(bool WaitForAll = true) 
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
					if (x.joinable())
						x.join();
				m_finished = true;
			}
		}

		/**
		*  Wait for the pool to empty before continuing.
		*  This does not call `std::thread::join`, it only waits until
		*  all jobs have finshed executing.
		*/
		void WaitAll() 
		{
			if (m_jobsLeft > 0)
			{
				std::unique_lock<std::mutex> lk(m_waitMutex);
				m_waitVar.wait(lk, [this]{ return this->m_jobsLeft == 0; });
				lk.unlock();
			}
		}
	};


} // namespace fpe

#endif //THREADPOOL_H