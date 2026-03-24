# Tests
Для проверки работоспособности реализованы следующие тесты:
1. Lock/Unlock
2. Проверка взаимного исключения
3. Проверка работы при высокой конкуренции
4. Блокировка потока
5. Корректность пробуждения потоков

```bash
Innoc3nt@Innoc3nt:~/Desktop/hse-mutithreading/4/build$ ./tests 
Running main() from ./googletest/src/gtest_main.cc
[==========] Running 5 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 5 tests from FutexMutexTest
[ RUN      ] FutexMutexTest.BasicLockUnlock
[       OK ] FutexMutexTest.BasicLockUnlock (0 ms)
[ RUN      ] FutexMutexTest.MutualExclusion
[       OK ] FutexMutexTest.MutualExclusion (4 ms)
[ RUN      ] FutexMutexTest.HighContention
[       OK ] FutexMutexTest.HighContention (38 ms)
[ RUN      ] FutexMutexTest.BlockingBehavior
[       OK ] FutexMutexTest.BlockingBehavior (50 ms)
[ RUN      ] FutexMutexTest.ManyWaiters
[       OK ] FutexMutexTest.ManyWaiters (61 ms)
[----------] 5 tests from FutexMutexTest (155 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test suite ran. (155 ms total)
[  PASSED  ] 5 tests.
```

