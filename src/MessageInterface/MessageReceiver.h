#ifndef __MESSAGE_RECEIVER_H__
#define __MESSAGE_RECEIVER_H__

#include "MessageInterface.h"

namespace MessageInterface
{
    class MessageReceiver
    {
    public:
        static void listen();

        MessageReceiver(const uint8_t *msg, size_t msgSize);
        ~MessageReceiver();

        void processMessage();

    private:
        uint8_t _message[128];
        size_t _msgSize;
        MessageReceiver();
    };
}

#endif