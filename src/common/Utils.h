#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <chrono>
#include <mutex>         
#include <condition_variable>
#include <array>

class RndUtils
{
public:

    using intRand = std::uniform_int_distribution<int>;
    using floatRand = std::uniform_real_distribution<float>;
    using RndEngine = std::default_random_engine;

    static void initRandomGen(const bool deterministic)
    {
        if (deterministic)
        {
            g_rGen.seed(1);
        }
        else
        {
            std::random_device dev;
            g_rGen.seed(dev());
        }
    }

    static RndEngine& getRndEngine()
    {
        return g_rGen;
    }

    // Instancing a rnd object on each call has (almost) the same cost of the old rand() 

    // randomize a number in 0,1
    static float randUniform()
    {
        floatRand rnd(0, 1);
        return rnd(g_rGen);
    }

    static float randFloatInRange(float min, float max)
    {
        floatRand rnd(min, max);
        return rnd(g_rGen);
    }

    static int randIntInRange(int min, int max)
    {
        intRand rnd(min, max);
        return rnd(g_rGen);
    }

    static std::chrono::milliseconds getRandomMillisecondsTime()
    {
        return std::chrono::milliseconds(RndUtils::randIntInRange(0, 100));
    }

private:
    static RndEngine g_rGen;
};

class DefaultTimer {
    int m_milliseconds;
public:
    DefaultTimer(long milliseconds) { m_milliseconds = milliseconds; }
    void operator()() const;
};



template<typename T, int CAPACITY>
class BoundedBuffer
{
public:
    std::array<T, CAPACITY> m_buffer;

    int m_front;    // Next index to read from
    int m_rear;     // Next index to write on
    int m_count;    // Number of elements active

    std::mutex m_lock;

    std::condition_variable m_notFullCondition;
    std::condition_variable m_notEmptyCondition;

    BoundedBuffer()
        : m_front(0)
        , m_rear(0)
        , m_count(0)
    {
    }

    ~BoundedBuffer()
    {
    }

    // Returns true if suceeded to deposit the value
    bool deposit(const T& data, bool wait = true)
    {
        std::unique_lock<std::mutex> l(m_lock);

        if (wait)
        {
            m_notFullCondition.wait(l, [this]() {return m_count != CAPACITY; });
        }

        if (m_count == CAPACITY)
        {
            return false;
        }

        m_buffer[m_rear] = data;
        m_rear = (m_rear + 1) % CAPACITY;
        ++m_count;

        l.unlock();
        m_notEmptyCondition.notify_one();
        return true;
    }

    bool empty() const { return m_count == 0; }

    bool fetch(T* outRes, bool wait = true) // Do not return reference !
    {
        std::unique_lock<std::mutex> l(m_lock);

        if (wait)
        {
            m_notEmptyCondition.wait(l, [this]() {return m_count != 0; });
        }

        if (m_count == 0)
        {
            return false;
        }

        if (outRes)
        {
            *outRes = m_buffer[m_front];
        }

        m_front = (m_front + 1) % CAPACITY;
        --m_count;

        l.unlock();
        m_notFullCondition.notify_one();
        return true;
    }
};



#endif
