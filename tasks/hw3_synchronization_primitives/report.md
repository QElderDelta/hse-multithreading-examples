## Домашнее задание
Вариант #1 -- задача buffered_channel.

### Результаты прогона тестов
[==========] Running 6 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 6 tests from Correctness
[ RUN      ] Correctness.Simple
[       OK ] Correctness.Simple (305 ms)
[ RUN      ] Correctness.Senders
[       OK ] Correctness.Senders (320 ms)
[ RUN      ] Correctness.Receivers
[       OK ] Correctness.Receivers (318 ms)
[ RUN      ] Correctness.SmallBuf
[       OK ] Correctness.SmallBuf (315 ms)
[ RUN      ] Correctness.BigBuf
[       OK ] Correctness.BigBuf (424 ms)
[ RUN      ] Correctness.Random
[       OK ] Correctness.Random (337 ms)
[----------] 6 tests from Correctness (2022 ms total)

[----------] Global test environment tear-down
[==========] 6 tests from 1 test suite ran. (2023 ms total)
[  PASSED  ] 6 tests.

### Результаты бенчмарка
Run on (8 X 1190 MHz CPU s)
Load Average: 0.52, 0.58, 0.59
***WARNING*** Library was built as DEBUG. Timings may be affected.
-----------------------------------------------------------------------------------------------
Benchmark                                                     Time             CPU   Iterations
-----------------------------------------------------------------------------------------------
Run/2/2/6/min_time:0.100/process_time/real_time            4070 ms        17578 ms            1
Run/10/4/4/min_time:0.100/process_time/real_time           1199 ms         6734 ms            1
Run/100000/4/4/min_time:0.100/process_time/real_time        318 ms         2094 ms            1