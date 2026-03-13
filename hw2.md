## BufferedChannel

.\build\buffered_channel\Debug\buffered_channel_test.exe

Output:

[==========] Running 6 tests from 1 test suite.
[ RUN      ] Correctness.Simple
[       OK ] Correctness.Simple
[ RUN      ] Correctness.Senders
[       OK ] Correctness.Senders
[ RUN      ] Correctness.Receivers
[       OK ] Correctness.Receivers
[ RUN      ] Correctness.SmallBuf
[       OK ] Correctness.SmallBuf
[ RUN      ] Correctness.BigBuf
[       OK ] Correctness.BigBuf
[ RUN      ] Correctness.Random
[       OK ] Correctness.Random

[  PASSED  ] 6 tests.



## UnbufferedChannel

.\build\unbuffered_channel\Debug\unbuffered_channel_test.exe

Output:

[==========] Running 6 tests from 2 test suites.

[ RUN      ] Correctness.Simple
[       OK ] Correctness.Simple
[ RUN      ] Correctness.Senders
[       OK ] Correctness.Senders
[ RUN      ] Correctness.Receivers
[       OK ] Correctness.Receivers
[ RUN      ] Correctness.BigBuf
[       OK ] Correctness.BigBuf

[ RUN      ] Block.Sender
[       OK ] Block.Sender
[ RUN      ] Block.Receiver
[       OK ] Block.Receiver

[  PASSED  ] 6 tests.
