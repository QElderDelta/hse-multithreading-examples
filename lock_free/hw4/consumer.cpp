#include "message_queue.h"
#include <iostream>

int main() {
    MessageQueue mq(false);
    mq.Subscribe(10);
    while(true) {
        auto data = mq.Receive();
        std::cout << "Получено: " << (char*)data.data() << std::endl;
    }
    return 0;
}