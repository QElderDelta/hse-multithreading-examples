#include "message_queue.h"
#include <iostream>
#include <thread>

int main() {
    MessageQueue mq(true, 1024);
    uint32_t type = 10;
    std::string msg = "Hello world!";
    while(true) {
        mq.Send(type, msg.c_str(), msg.size() + 1);
        std::cout << "Отправлено: " << msg << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}