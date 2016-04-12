#include <functional>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

#include <softwear/thread_pool.hpp>

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 1
#include <cassert>

using std::atomic;

using std::this_thread::sleep_for;
using std::chrono::milliseconds;

using softwear::thread_pool;

struct state {
  atomic<size_t> call_count{0}, run_count{0};
};

static atomic<size_t> moved{0}, copied{0};
struct callable {

  callable() {}

  callable(callable&&) { moved++;   }
  callable& operator=(callable&&) { moved++; return *this; }

  callable(const callable&) { copied++; }
  callable& operator=(const callable&) { copied++; return *this; }

  void operator()(state &s) {
    // Note: Using the state to check whether passing
    // references works; if thread stores a decay copy, we
    // will see different counts on the test thread.
    s.run_count++;
    sleep_for(milliseconds{40});
    s.run_count--;
    s.call_count++;
  }
} f;

int main() {
  auto fr = std::ref(f);

  // Test basic non-threaded invocation
  {
    state s0;
    f(s0);
    assert(s0.call_count == 1);
    assert(s0.run_count == 0);
    assert(moved == 0);
    assert(copied == 0);
  }

  auto check_run = [&](thread_pool &p, state &s, size_t thrno) {
    // callables should have initialized but should still be
    // waiting
    sleep_for(milliseconds{5});

    if (thrno == 0) assert(s.run_count > 0);
    else            assert(s.run_count == thrno);
    assert(!p.idle());

    p.join(); // wait for it's completion
    if (thrno == 0) assert(s.call_count > 0);
    else            assert(s.call_count == thrno);
    assert(s.run_count == 0);
    assert(moved == 0);
    assert(copied == 0);
  };

  // Test with default size, by supplying size=0 (should at least start one)
  {
    state s1, s2;
    thread_pool p{0, fr, s1};
    check_run(p, s1, 0);
    p.run(fr, s2); // Restart
    check_run(p, s2, 0);
  }

  // Test with default size without auto run
  {
    state s;
    thread_pool p;
    p.run(fr, s); // Restart
    check_run(p, s, 0);
  }

  // Test with size set to zero without auto run
  {
    state s;
    thread_pool p{0};
    p.run(fr, s); // Restart
    check_run(p, s, 0);
  }

  // Test with specific size
  {
    state s1, s2;
    thread_pool p{70, fr, s1};
    check_run(p, s1, 70);
    p.run(fr, s2); // Restart
    check_run(p, s2, 70);
  }

  // Test with specific size without auto run
  {
    state s;
    thread_pool p{70};
    p.run(fr, s); // Restart
    check_run(p, s, 70);
  }

  assert(false);

  std::cerr << "Tests ran successfully!\n";
  return 0;
}
