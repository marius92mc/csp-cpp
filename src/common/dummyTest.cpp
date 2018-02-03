#include <iostream>


#define select_0(channel1, readFromChannel1, outVar1, channel2, readFromChannel2, outVar2, channel3, readFromChannel3, outVar3,  guiChannel, readFromGuiChannel, outGuiChannel) \
{  \
    while (true) {  \
        if (readFromChannel1)   \
        {   \
            if (channel1.read(outVar1, false))  \
            { \
             printf("1\n"); \
             break;  \
            } \
        }   \
    } \
}


int main()
{
    return 0;
}