#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

template <class T>
class ChannelUnbounded {
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable conditionalVariable;

public:
    void write(const T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(value);
        conditionalVariable.notify_one();
    }

    void read(T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        conditionalVariable.wait(
            lock,
            [&]() {
                return !queue.empty();
            }
        );
        value = queue.front();
        queue.pop();
    }
};


ChannelUnbounded<std::string> myChannel;


class Example {
  public:
    void process(const int& id) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        myChannel.write("message " + std::to_string(id));
    }
};


int main() {
    Example example;
    std::thread thread1(&Example::process, &example, 1);
    std::thread thread2(&Example::process, &example, 2);

    std::string message1, message2;

    myChannel.read(message1);
    myChannel.read(message2);

    if (thread1.joinable()) {
        thread1.join();
    }
    if (thread2.joinable()) {
        thread2.join();
    }

    std::cout << message1 << "\n" << message2 << "\n";

    return 0;
}
