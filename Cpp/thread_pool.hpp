#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP


#include <atomic>
#include <future>
#include <thread>
#include <mutex>
#include <list>

#include "prelude.h"

// template <typename Fun, typename... Args, size_t... index>
// fn __fun_call(Fun&& fun, const tuple<Args...>& args, const
// std::_Index_tuple<index...>&)
//     ->decltype(fun(std::get<index>(args)...)) {
//   return fun(std::get<index>(args)...);
// }

// template <typename Fun, typename... Args, size_t... index>
// fn __fun_call(Fun&& fun, tuple<Args...>&& args, const
// std::_Index_tuple<index...>&)
//     ->decltype(fun(std::get<index>(args)...)) {
//   return fun(std::get<index>(args)...);
// }

class TaskExecutor {
 public:
  // using Task = Box<Fn<void()>>;
  using Task = Fn<void()>;

 public:
  TaskExecutor(int thread_num = std::thread::hardware_concurrency()) {
    thread_num_ = (thread_num >= 4 ? thread_num : 4);
    stop_ = false;
    for (int i = 0; i < thread_num_; ++i) {
      threads_.emplace_back([this] {
        while (not stop_.load()) {
          Task task;
          {
            std::unique_lock<std::mutex> lock(task_mutex_);
            if (stop_.load()) {
              break;
            } else if (tasks_.size() == 0) {
              have_task_cond_.wait(lock);
              continue;
            } else {
              task = std::move(tasks_.front());
              tasks_.pop_front();
            }
          }
          task();
        }
      });
    }
  }

  ~TaskExecutor() { stop(); }

  fn stop()->void {
    stop_.store(true);
    have_task_cond_.notify_all();
    for (auto& trd : threads_) {
      if (trd.joinable()) trd.join();
    }
  }

  template <typename Fun, typename... Args>
  fn async_call(Fun&& fun, Args&&... args)
      ->std::future<decltype(fun(std::forward<Args>(args)...))> {
    using Ret = decltype(fun(args...));
    using PackagedTask = std::packaged_task<Ret()>;

    var binded = std::bind(std::forward<Fun>(fun), std::forward<Args>(args)...);

    var p_task =
        make_shared<PackagedTask>([fun = std::move(binded)] { return fun(); });

    var fut = p_task->get_future();
    {
      std::lock_guard<std::mutex> guard(task_mutex_);
      size_t pre_size = tasks_.size();
      tasks_.emplace_back([p_task = std::move(p_task)] { (*p_task)(); });
      if (pre_size == 0) {
        have_task_cond_.notify_one();
      }
    }
    return fut;
  }

  template <typename Fun, typename... Args>
  fn sync_call(Fun&& fun, Args&&... args)
      ->decltype(fun(std::forward<Args>(args)...)) {
    return async_call(std::forward<Fun>(fun), std::forward<Args>(args)...)
        .get();
  }

  fn thread_num()->int { return thread_num_; }

 private:
  std::mutex task_mutex_;
  std::atomic_bool stop_;
  int thread_num_;
  vector<std::thread> threads_;
  std::condition_variable have_task_cond_;
  std::list<Task> tasks_;
};

#endif