#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "prelude.h"
#include <atomic>
#include <future>
#include <thread>
#include <deque>
#include <list>
#include <mutex>

class TaskExcutor { 
public:
	// using Task = Box<Fn<void()>>;
	using Task = Fn<void()>;
public:
	TaskExcutor(int thread_num = std::thread::hardware_concurrency()) {
		thread_num_ = thread_num;
		stop_ = false;
		for (int i = 0; i < thread_num; ++i) {
			threads_.emplace_back([this] {
				while (not stop_) {
					{
						std::unique_lock<std::mutex> lock(task_mutex_);
						cond_.wait(lock, [this] { return stop_ or tasks_.size() != 0; });
					}
					if (stop_.load()) {
						break;
					}
					Task task = std::move(tasks_.front());
					tasks_.pop_front();
					task();
				}
			});
		}
	}

	~TaskExcutor() {
		stop();
	}

	fn stop() -> void {
		stop_.store(true);
		for (auto& trd : threads_) {
			if (trd.joinable())
				trd.join();
		}
	}

	template <typename Fun, typename... Args>
	fn execute(Fun&& fun, Args&&... args) -> std::future<decltype(fun(args...))> {
		using Ret = decltype(fun(args...));
		auto binded = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);
		Box<std::packaged_task<Ret()>> wrapper_ptr(new std::packaged_task<Ret()>([fun=std::move(binded)]{ return fun(); }));
		auto fut = wrapper_ptr->get_future();
		{
		   	std::lock_guard<std::mutex> guard(task_mutex_);
		   	tasks_.push_back([wrapper_ptr=std::move(wrapper_ptr)] { (*wrapper_ptr)(); });
		   	cond_.notify_one();
		}
		// (*wrapper_ptr)();
		return fut;
	}


	// template <typename Fun, typename... Args>
	// inline fn execute(Fun&& fun, Args&&... args) -> std::future<decltype(fun(std::forward<Args>(args)...))> {
	// 	return execute([fun=std::move(fun), args...] { return fun(std::forward<Args>(args)...); });
	// }


private:
	std::mutex task_mutex_;
	std::atomic_bool stop_;
	int thread_num_;
	vector<std::thread> threads_;
	std::condition_variable cond_;
	std::list<Task> tasks_;
};

#endif