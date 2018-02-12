#pragma once
#ifndef AUTOGENERATED_H
#define AUTOGENERATED_H

// ==================== AUTOGENERATED SOURCE CODE - should be probably moved to a different file that gathers all these =============

// The parameters size is dynamic depending on the select function used. Please check the comment below at function call to understand
// why
// Datatypes, max size should be copied from the user given code that declares the channels
// Same for variables names, we have to create a convetion for the autogenerated code
// DO NOT COPY USER'S COMMENTS
// "\" SHOULD BE THE LAST CHARACTER NO SPACE OR SOMETHING AFTER IT !!
#define select_0(channel1, readFromChannel1, outVar1, channel2, readFromChannel2, outVar2, channel3, readFromChannel3, outVar3,  guiChannel, readFromGuiChannel, outGuiChannel) \
{  \
    while (true) \
    {  \
        if (readFromChannel1)   \
        {   \
            if (channel1.read(&outVar1, false))  \
            {   \
                printf("1\n"); \
                break;  \
            }   \
        }   \
        else \
        { \
        } \
        if (readFromChannel2) \
        { \
            if (channel2.read(&outVar2, false)) \
            { \
                printf("2\n");\
                break; \
            } \
        } \
        else \
        { \
            \
        } \
        if (readFromGuiChannel) \
        { \
            if (guiChannel.read(&outGuiChannel, false)) \
            { \
                printf("GUI\n"); \
                break; \
            } \
        } \
        else \
        { \
           \
        } \
        if (readFromChannel3) \
        { \
           \
        } \
        else \
        { \
            if (channel3.write(outVar3, false)) \
            { \
                printf("Succeeded to write on channel 3\n"); \
                break; \
            } \
        } \
    } \
} 

// =========================== END AUTOGENERATED SOURCE CODE ==================================

#endif