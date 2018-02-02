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

void process(ChannelUnbounded<std::string>& channelToWriteOn, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write("message " + std::to_string(id));
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
    ChannelUnbounded<std::string> channel1, channel2;
    std::thread thread1(&process, std::ref(channel1), 1);
    std::thread thread2(&process, std::ref(channel2), 2);
    std::string message1, message2;

    // Simulation of Golang's `select` statement
    while (true) 
    {
        if (channel1.read(message1, false)) 
        {
            std::cout << "Received from channel1.\n";
            break;
        }
        else if (channel2.read(message2, false)) {
            std::cout << "Received from channel2.\n";
            break;
        }
    }

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
