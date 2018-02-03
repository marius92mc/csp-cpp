#include "Utils.h"
#include <thread>
#include <chrono>

RndUtils::RndEngine RndUtils::g_rGen;

void DefaultTimer::operator()() const
{
    std::this_thread::sleep_for(std::chrono::milliseconds(m_milliseconds));
}

