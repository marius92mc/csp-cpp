#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <random>
#include <chrono>

#include "common/Utils.h"
#include "common/ChannelUnbounded.h"
#include "common/ChannelBounded.h"

struct MyStruct
{
    float x, y;
};

void process1(ChannelUnbounded<std::string, 100>& channelToWriteOn, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());// RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write("message " + std::to_string(id));
}

void process2(ChannelUnbounded<int, 50>& channelToWriteOn, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write(id);
}


void process3(ChannelBounded<MyStruct>& channelToReadFrom, const int id)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());

    MyStruct res;
    channelToReadFrom.read(&res);
}


void guiSimulation(ChannelUnbounded < bool , 10> & channelToWriteOn)
{
    // Simulate gui events and at some poin...exit because of a quit event !
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write(true);
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
    ChannelUnbounded<std::string, 100> channel1;
    ChannelUnbounded<int, 50>    channel2;
    ChannelUnbounded<bool, 10>   channel_GuiSim;

    // Note that this is a bounded channel
    ChannelBounded<MyStruct> channel_commTest;

    // Main thread writes something to the commTest channel
    channel_commTest.write(MyStruct{ 1,2 });

    std::thread thread1(&process1, std::ref(channel1), 1);
    std::thread thread2(&process2, std::ref(channel2), 2);
    
    // Process3 will consume something from the commTest channel.
    std::thread thread3(&process3, std::ref(channel_commTest), 3);
    
    // Waits for q quit event
    std::thread thread_gui(&guiSimulation, std::ref(channel_GuiSim));


    // Init messages with empty stuff
    std::string message1;
    int message2 = -1;
    MyStruct newWrite{ 3,4 };

    // Simulation of Golang's `select` statement

    select {
        case message1 <- channel1:
        {
            printf("1");
        }   
        case message2 <- channel2:
        {
            printf("2");
        }

        case channel3 <- message3: // Note that this is a write instruction !!
        {
            printf("Succeeded to write on channel 3\n");
        }

        case <- channel_GuiSim:
        {
            printf("Gui");
        }

        default:
        {
            print ("nothing");
        }

        // TODO: select should also support default for non blocking stuff see: https://gobyexample.com/non-blocking-channel-operations
        // Make an example + support with that situation too
    }
    
   // THE DEFINE CODE WILL BE IN A DIFFERENT FILE
   // We can't do it as a function since we'll lose user's context
   // select_X where X define the X'th encountered selec in this file


    /*select_0(channel1, true, &message1,
        channel2, true, &message2,
        channel_commTest, false, &newWrite,
        channel_GuiSim, true, nullptr);
    */
        // Check what happened during selec
   std::cout << "values for out variables after selec " << "One: " << message1.c_str() << " Two : " << message2 << std::endl;


    // Wait for all threads as a good client
    {
        if (thread1.joinable())
        {
            thread1.join();
        }
        if (thread2.joinable())
        {
            thread2.join();
        }
        if (thread3.joinable())
        {
            thread3.join();
        }
        if (thread_gui.joinable())
        {
            thread_gui.join();
        }
    }
}

int main()
{
    RndUtils::initRandomGen(false);

    runSelectExample();
    return 0;
}
