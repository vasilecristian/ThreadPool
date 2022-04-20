#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <functional>
#include <condition_variable>

namespace tp 
{
	/**
	*  Simple ThreadPool that creates `threadCount` threads upon its creation,
	*  and pulls from a queue to get new jobs. The default is 10 threads.
	*  This class requires a number of c++11 features be present in your compiler.*/
	class ThreadPool 
	{
	public:
		ThreadPool(unsigned int threadCount);

		/** JoinAll on deconstruction */
		~ThreadPool();

		/** Get the number of threads in this pool */
		uint32_t Size() const;

		/** Get the number of jobs left in the queue.*/
		size_t JobsRemaining();

		/**
		*  Add a new job to the pool. If there are new jobs in the queue,
		*  a thread is woken up to take the job. If all threads are busy,
		*  the job is added to the end of the queue.*/
		void AddJob(std::function<void(void)> job);

		/**
		*  Join with all threads. Block until all threads have completed.
		*  Params: WaitForAll: If true, will wait for the queue to empty
		*          before joining with threads. If false, will complete
		*          current jobs, then inform the threads to exit.
		*  The queue will be empty after this call, and the threads will
		*  be done. After invoking `ThreadPool::JoinAll`, the pool can no
		*  longer be used. If you need the pool to exist past completion
		*  of jobs, look to use `ThreadPool::WaitAll`.*/
		void JoinAll(bool WaitForAll = true);

		/**
		*  Wait for the pool to empty before continuing.
		*  This does not call `std::thread::join`, it only waits until
		*  all jobs have finshed executing.
		*/
		void WaitAll();

	private:
		uint32_t m_threadCount;
		std::vector<std::thread> m_threads;
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
		void Task();

		/**
		*  Get the next job; pop the first item in the queue,
		*  otherwise wait for a signal from the main thread.
		*/
		std::function<void(void)> NextJob();

	};


} // namespace tp

#endif //THREADPOOL_H
