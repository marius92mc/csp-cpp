#include <iostream>      
#include <thread>        


#include "common/Utils.h"

void consumer(int id, BoundedBuffer<int,200> & buffer) {
    for (int i = 0; i < 50; ++i) {
        int value = -1;
        buffer.fetch(&value);
        std::cout << "Consumer " << id << " fetched " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

void producer(int id, BoundedBuffer<int, 200>& buffer) {
    for (int i = 0; i < 75; ++i) {
        buffer.deposit(i);
        std::cout << "Produced " << id << " produced " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    BoundedBuffer<int, 200> buffer;

    std::thread c1(consumer, 0, std::ref(buffer));
    std::thread c2(consumer, 1, std::ref(buffer));
    std::thread c3(consumer, 2, std::ref(buffer));
    std::thread p1(producer, 0, std::ref(buffer));
    std::thread p2(producer, 1, std::ref(buffer));

    c1.join();
    c2.join();
    c3.join();
    p1.join();
    p2.join();

    return 0;
}
