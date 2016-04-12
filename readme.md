# thread_pool

C++11 library for managing a number of threads and executing
a function in parallel in all of the threads.

## Using

Add this repository as a submodule, add `include/` to your
include paths.

```c++
#include <softwear/thread_pool.hpp>
using softwear::thread_pool;

## Reference

See the header file for a detailed API documentation.

In general the API is thread safe in regards to the worker
threads, but the controlling API may only be called by one
thread at the same time.

```c++
struct thread_pool {
  /// Initialize the pool and start the worker threads
  thread_pool(size_t size);
  thread_pool(); /// Auto detecting the number of CPUs
  
  /// Initialize the pool, the workers and run a job
  template<typename F, typename... Args>
  thread_pool(size_t size, F f, Args&&... args);

  /// Wait until all workers are idle then stop the threads.
  ~thread_pool();
    
  /// Move construction and assignment is supported
  thread_pool(thread_pool&&) = default;
  thread_pool& operator=(thread_pool&&) = default;

  /// Copy construction and assignment is not
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

  /// Run a job; waits until the pool is idle before
  // running the job; usually stores a copy of the Callable, except when
  /// std::ref is used; other than the std::thread this will
  /// *not* store decay copies of arguments that are references
  template<typename Callable, typename... Args>
  void run(Callable f, Args&&... args);

  /// Check whether the last job has compleeted on all threads
  bool idle();

  void join(); /// Wait till the pool is idle


  size_t size(); /// How many threads there are

  Iterator begin(); /// Iterating the threads
  Iterator end();
}
```

## Testing

Run `make`

# LICENSE

Written by (karo@cupdev.net) Karolin Varner, for Softwear, BV (http://nl.softwear.nl/contact):
You can still buy me a Club Mate. Or a coffee.

Copyright © (c) 2016, Softwear, BV.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of the Karolin Varner, Softwear, BV nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Softwear, BV BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
