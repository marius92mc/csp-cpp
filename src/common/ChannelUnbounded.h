#pragma once
#ifndef CHANNEL_UNBOUNDED_H
#define CHANNEL_UNBOUNDED_H


// TODO: make this respect the size property
template <class T, int MAXSIZE>
class ChannelUnbounded 
{
    BoundedBuffer<T,MAXSIZE> m_buffer;
    bool m_isClosed;

public:
    ChannelUnbounded() {
        m_isClosed = false;
    }

    void close() {
        m_isClosed = true;
    }

    bool isClosed() {
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
        if (m_isClosed) {
            throw std::logic_error("Cannot write to a closed channel.\n");
        }
        
        const bool res = m_buffer.deposit(value, wait);
        return res;
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
        const bool res = m_buffer.fetch(value, wait);
        return res;
    }
};

#endif
