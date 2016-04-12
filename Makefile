.PHONY: run-test clean

run-test: test
	./test

test: tests.cc include/softwear/thread_pool.hpp
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(CPPFLAGS) -pthread -DDEBUG=1 -g -O3 -Wall -Wextra -Wpedantic -Werror -std=c++11 -I"$(PWD)"/include tests.cc -o test

clean:
	rm test
