#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <random>
#include <chrono>


#include "common/Utils.h"
#include "common/Channel.h"


// The parameters size is dynamic depending on the select function used. Please check the comment below at function call to understand
// why
void select_0(ChannelUnbounded<std::string>& channel1,
    const bool readFromChannel1, // Read or write ?
    std::string& outVar1,
    ChannelUnbounded <int>& channel2,
    const bool readFromChannel2,
    int& outVar2)
{
    // TODO: run this with two options: spinning lock or as a task lock.
    // In task lock, create it as a TBB or simple THREAD to avoid the dependency if TBB is not needed elsewhere and wait for it.
    // In spin lock do exactly what it does now
    while (true)
    {
        if (readFromChannel1)
        {
            if (channel1.read(outVar1, false))
                break;
        }

        if (readFromChannel2)
        {
            if (channel2.read(outVar2, false))
                break;
        }
    }
}


void process1(ChannelUnbounded<std::string>& channelToWriteOn, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());// RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write("message " + std::to_string(id));
}

void process2(ChannelUnbounded<int>& channelToWriteOn, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write(id);
}

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
void runSelectExample()
{
    ChannelUnbounded<std::string> channel1;
    ChannelUnbounded<int>    channel2;
    std::thread thread1(&process1, std::ref(channel1), 1);
    std::thread thread2(&process2, std::ref(channel2), 2);
    std::string message1;
    int message2 = -1;

    // Simulation of Golang's `select` statement
    /*
    select {
    case m1 <- c1
    case m2 <- c2
    }

    ====> TRANSLATES TO A FUNCTION CALL and a function definition at the beggining of this file (or potentioanlly another cpp linked file extern from users input)
    // select_X where X define the X'th encountered select in this file
    */
    select_0(channel1, true, message1, channel2, true, message2);

    std::cout << "values after select " << "One: " << message1.c_str() << " Two : " << message2 << std::endl;

    if (thread1.joinable())
    {
        thread1.join();
    }
    if (thread2.joinable())
    {
        thread2.join();
    }
}

int main()
{
    RndUtils::initRandomGen(false);

    runSelectExample();
    return 0;
}
