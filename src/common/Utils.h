#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <chrono>

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


#endif