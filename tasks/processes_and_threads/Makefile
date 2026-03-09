CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -pthread
LDFLAGS = -lgtest -lgtest_main -lbenchmark -lpthread

all: test benchmark

test: test.cpp apply_function.h
	$(CXX) $(CXXFLAGS) test.cpp -o test_exe $(LDFLAGS)

benchmark: benchmark.cpp apply_function.h
	$(CXX) $(CXXFLAGS) benchmark.cpp -o bench_exe $(LDFLAGS)

clean:
	rm -f test_exe bench_exe
