#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <utility>

namespace softwear {

/// A group of threads used to execute the same function on
/// many threads.
///
/// Initializing the thread pool automatically starts up all
/// the threads; destroying it will wait for the current job
/// to finish and stop the threads.
///
/// Jobs may not throw exceptions; if a worker throws an
/// exception, the program will exit.
///
/// Note that while starting and stopping stopping the threads
/// and running jobs is thread safe in relation to the
/// worker threads, the controlling methods in this interface are
/// not thread safe: There may only be one thread calling
/// the destructor, run() or join().
/// begin(), end() and size() are thread safe for read only
/// access.
class thread_pool : private std::vector<std::thread> {
  typedef std::vector<std::thread> super_t;
  super_t *super = (super_t*)this;

  std::atomic<bool> quit{false}; /// Signals the workers they should quit
  std::function<void()> job;

  /// Number of workers not idle; that is also the case
  /// while they are starting up
  std::atomic<size_t> working;

  /// Instruction index, so threads can check whether they
  /// have
  std::atomic<size_t> instruction_no{0};
  std::mutex on_run_mtx, on_idle_mtx;
  std::condition_variable
    on_idle, /// Signal join() that one thread has become idle
    on_run;  /// Signal threads they should quit/execute an instruction


  void worker_loop() noexcept {
    size_t executed_instructions = 0;

    // Need the on_run_lock here so we are definitely
    // waiting before working becomes zero -> join() can
    // exit -> run can notify the lock
    std::unique_lock<std::mutex> on_run_lock(on_run_mtx);
    working--;
    on_idle.notify_all();

    while (true) {
      on_run.wait(on_run_lock);
      // Not interested here, we just want to start up all
      // workers in parallel
      on_run_lock.unlock();

      if (quit) {
        break;

      } else if (instruction_no > executed_instructions) {
        // working++; – Not necessary, run() has already done this for us

        job();

        // Make sure we have the on_run_lock again before
        // idle can finish, so there is no way run() can
        // notify on_run before this thread is really
        // waiting
        on_run_lock.lock();
        working--;
        executed_instructions++;
        on_idle.notify_all();
      }
    }
  }

public:
  /// Create a thread pool with as many cpus as this system
  /// has.
  /// If that number could not be detected, four threads will be started.
  thread_pool() : thread_pool{std::thread::hardware_concurrency()} {}

  /// Create thread pool with a given number of threads;
  /// If zero is given, four threads will be started.
  thread_pool(size_t size) : working{size} {
    // Note: We set working to the number of threads, since
    // threads only become idle once they are really started
    // up
    if (size == 0) size = 4;
    reserve(size);
    working = size;
    for (size_t i=0; i<size; i++) emplace_back(&thread_pool::worker_loop, this);
  }

  /// Create thread pool with a given number of threads and
  /// run a function.
  /// If the number of threads given is zero, four threads
  /// will be used.
  template<typename F, typename... Args>
  thread_pool(size_t size, F f, Args&&... args) : thread_pool{size} {
    run<F, Args...>(f, std::forward<Args>(args)... );
  }

  ~thread_pool() {
    join();
    quit=true;
    on_run.notify_all();
    for (auto &t : *this) t.join();
  }

  thread_pool(thread_pool&&) = default;
  thread_pool& operator=(thread_pool&&) = default;

  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

  /// Run the given function with the applied parameters as
  /// a job.
  ///
  /// Other than std::thread, this will not store decay
  /// copies of either the given callable or any of it's
  /// parameters, so you need to make sure all of them stay
  /// alive until this job finishes.
  template<typename F, typename... Args>
  void run(F f, Args&&... args) {
    join();

    std::unique_lock<std::mutex> on_run_lock(on_run_mtx);

    job = [f, &args...]() {
      return f( std::forward<Args>(args)... );
    };
    instruction_no++;
    // You'd expect working to be incremented in the
    // workers, but we set it here so join() is guaranteed
    // to wait after this exits; otherwise join() might have
    // been called before any of the workers had a chance to
    // increment working
    working = size();

    on_run.notify_all();

    /// Now we're freeing the on_run_lock (by scope) so now
    /// all the workers are actually starting.
  }

  /// Waits until the threads are idle
  void join() {
    std::unique_lock<std::mutex> on_idle_lock(on_idle_mtx);
    while (!idle()) on_idle.wait(on_idle_lock);
  }

  /// Checks whether all the workers are idle
  bool idle() { return working == 0; }

  /// Number of threads in this pool
  auto size() -> decltype(super->size()) { return super->size(); }
  auto begin() -> decltype(super->begin()) { return super->begin(); }
  auto end() -> decltype(super->begin()) { return super->end(); }
};

} // namespace softwear
