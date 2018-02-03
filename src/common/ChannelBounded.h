#pragma once
#ifndef CHANNEL_BOUNDED_H
#define CHANNEL_BOUNDED_H

#include <thread>
#include <mutex>
#include <condition_variable>


// By default sends and receives block until both the sender and receiver are ready : https://gobyexample.com/channels
template <class T>
class ChannelBounded
{
    T m_element;
    std::mutex m_mutex;

    // Conditions that enables us to write / read stuff
    // Writer waits for notFull (if needed) and signals notEmpty after it writes
    // reader waits for notEmpty (if needed) and signals notFull after it read
    std::condition_variable m_notFullCondition;
    std::condition_variable m_notEmptyCondition;

    bool m_isClosed = false;
    bool m_isEmpty = true;

public:
    ChannelBounded() 
    {
        m_isClosed = false;
        m_isEmpty = true;
    }

    void close() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_isClosed = true;
        m_notFullCondition.notify_all();
        m_notEmptyCondition.notify_all();
    }

    bool isClosed() 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_isClosed;
    }

    /**
    * @brief
    *        Write a value to a channel.
    * @param value
    *        The item that will be added to the channel.
    */
    bool write(const T& value, const bool wait = true) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_isClosed) 
        {
            throw std::logic_error("Cannot write to a closed channel.\n");
        }

        if (wait)
        {
            m_notFullCondition.wait(lock, [this]() { return m_isClosed || m_isEmpty == true; });
        }

        if (!m_isEmpty)
        {
            return false;
        }

        m_element = value;
        m_isEmpty = false;
        m_notEmptyCondition.notify_one();
        return true;
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
    bool read(T* value, bool wait = true) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (wait) 
        {
            m_notEmptyCondition.wait(lock, [this]() { return m_isClosed || m_isEmpty == false; });
        }

        if (m_isEmpty)
        {
            return false;
        }

        // Write the value in output variable
        if (value != nullptr)
        {
            *value = m_element; // Need to return a copy since this internal will become available for further writes ! If you need to use shallow copies, use T as a pointer !
        }

        m_isEmpty = true;
        return true;
    }
};

#endif
