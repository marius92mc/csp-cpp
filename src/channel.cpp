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
    bool closed;

public:
    ChannelUnbounded() {
        closed = false;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mutex);
        closed = true;
        conditionalVariable.notify_all();
    }

    bool isClosed() {
        std::unique_lock<std::mutex> lock(mutex);
        return closed;
    }

    /**
     * @brief
     *        Write a value to a channel.
     * @param value
     *        The item that will be added to the channel.
     */
    void write(const T& value) {
        std::unique_lock<std::mutex> lock(mutex);
        if (closed) {
            throw std::logic_error("Cannot write to a closed channel.\n");
        }
        queue.push(value);
        conditionalVariable.notify_one();
    }

    /**
     * @brief
     *        Read the first value from a channel.
     *        This could be used to process items until a channel
     *        is closed and drained.
     * @param
     *        The variable in which it will be put the value from the
     *        channel.
     * @return
     *        `true`, if an item was received.
     *        `false`, if there was no item to read from the channel, i. e.
     *                 the channel was empty.
     */
    bool read(T& value) {
        std::unique_lock<std::mutex> lock(mutex);

        conditionalVariable.wait(
            lock,
            [&]() {
                return closed || (!queue.empty());
            }
        );

        if (queue.empty()) {
            return false;
        }

        value = queue.front();
        queue.pop();
        return true;
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
