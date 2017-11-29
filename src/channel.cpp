#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "tbb/include/tbb/tbb.h"

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
     *      value
     *        The variable in which it will be put the value from the
     *        channel.
     *      wait
     *        Controls whether it blocks until an item is available
     *        or the channel is closed.
     *        By default is set to `true` to be blocking.
     * @return
     *        `true`, if an item was received.
     *        `false`, if there was no item to read from the channel, i. e.
     *                 the channel was empty.
     */
    bool read(T& value, bool wait = true) {
        std::unique_lock<std::mutex> lock(mutex);

        if (wait) {
            conditionalVariable.wait(
                lock,
                [&]() {
                    return closed || (!queue.empty());
                }
            );
        }

        if (queue.empty()) {
            return false;
        }

        value = queue.front();
        queue.pop();
        return true;
    }
};


ChannelUnbounded<std::string> myChannel;
ChannelUnbounded<std::string> channel1, channel2;

// Variable shared by all threads/tasks in order to check if the task was finished.
int kSharedVariable = 0;


class Example {
  public:
    void process(const int& id) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        myChannel.write("message " + std::to_string(id));
    }

    void process1(const int& id) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        channel1.write("message " + std::to_string(id));
        kSharedVariable = id;
    }

    void process2(const int& id) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        channel2.write("message " + std::to_string(id));
        kSharedVariable = id;
    }
};

/**
 * @brief
 * 		  This function shows an example of a Golang `select` simulation
 * 		  without using operator overloadings, just plain C++ statements.
 *
 * 		  The threads creation section is needed because they call the
 * 		  methods that write on the channels.
 * 		  The last section is needed because of the threads safely termination.
 *
 * 		  The `select` behavior is comprised by the loop.
 */
void runSelectExample() {
    Example example;
    std::thread thread1(&Example::process1, &example, 1);
    std::thread thread2(&Example::process2, &example, 2);
    std::string message1, message2;

    // Simulation of Golang's `select` statement
    while (true) {
        if (channel1.read(message1, false)) {
            std::cout << "Received from channel1.\n";
            break;
        } else if (channel2.read(message2, false)) {
            std::cout << "Received from channel2.\n";
            break;
        }
    }

    if (thread1.joinable()) {
        thread1.join();
    }
    if (thread2.joinable()) {
        thread2.join();
    }
}

class TbbExample {
  public:
    TbbExample() {
    }
    
    ~TbbExample() {
    }

    void runTbbExample() {
        Example example;
        tbb::task_group taskGroup;
        while (true) {
            taskGroup.run(example.process1(1));
            taskGroup.run(example.process2(2));
            taskGroup.wait();
            // kSharedVariable is 0 initially, shared between all tasks
            if (kSharedVariable != 0) {
                std::cout << "First task finished was " << id << ". \n";
                // Kill all other tasks because the first one already received
                tbb::task::self().cancel_group_execution();
                break;
            }
        }
    }
};

int main() {
    std::string input;

    std::cout << "\n1. Select example. \n"
                   "2. Without Select. \n"
                   "3. With tbb. \n";
    std::cin >> input;
    if (input == "1") {
        runSelectExample();
        return 0;
    }
    if (input == "3") {
        TbbExample tbbExample;
        tbbExample.runTbbExample();
        return 0;
    }

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
