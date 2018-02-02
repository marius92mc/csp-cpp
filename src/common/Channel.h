#pragma once

template <class T>
class ChannelUnbounded 
{
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
