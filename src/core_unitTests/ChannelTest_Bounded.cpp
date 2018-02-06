#include "common/ChannelBounded.h"
#include "common/Utils.h"
#include <assert.h>

void thread_writer(ChannelBounded<int>& channelToWriteOn, const int& value)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    channelToWriteOn.write(value);
}

void thread_reader(ChannelBounded<int>& channelToReadFrom, int* value)
{
    std::this_thread::sleep_for(RndUtils::getRandomMillisecondsTime());
    assert(channelToReadFrom.read(value));
}


int main()
{
    ChannelBounded<int> sharedChannel;

    // Unit testing channel
    for (int i = 0; i < 10; i++)
    {
        const int inputValue = i;
        int outputValue = 0;
        std::thread tWriter(thread_writer, std::ref(sharedChannel), inputValue);
        std::thread tReader(thread_reader, std::ref(sharedChannel), &outputValue);

        if (tWriter.joinable())
        {
            tWriter.join();
        }

        if (tReader.joinable())
        {
            tReader.join();
        }

        assert(outputValue == i);
    }

    return 0;
}
