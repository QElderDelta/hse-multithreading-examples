#### Тесты
`g++ -std=c++17 -O2 -pthread test_apply_function.cpp -lgtest -lgtest_main -o tests
./tests`

#### Бенчмарк
`g++ -std=c++17 -O2 -pthread bench_apply_function.cpp -lbenchmark -lbenchmark_main -o bench
./bench`