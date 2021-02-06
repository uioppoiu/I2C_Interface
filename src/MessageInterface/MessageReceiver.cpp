#include <Arduino.h>
#include "MessageReceiver.h"
#include "MessageCallback.h"
#include "../I2CInterface/I2CInterface.h"
#include "Endian.h"

void MessageInterface::MessageReceiver::listen()
{
    // Serial.printf("[%s:%d] Buffer size : %d\n", __FUNCTION__, __LINE__, i2cReadBufferIdx);
    // if(i2cReadBufferIdx == 0) return;
    // for (int i = 0; i < i2cReadBufferIdx; i++)
    // {
    //     Serial.printf("%d(%d)", i, (int)i2cReadBuffer[i]);
    //     Serial.print(' ');
    // }
    // Serial.println();

    const bool begin = (I2CInterface::i2cReadBufferIdx >= 7) &&
                       (memcmp(I2CInterface::i2cReadBuffer, "<BEGIN>", 7) == 0);
    if (begin == false)
        return;

    const bool end = (I2CInterface::i2cReadBufferIdx >= 5) &&
                     (memcmp(I2CInterface::i2cReadBuffer + I2CInterface::i2cReadBufferIdx - 5, "<END>", 5) == 0);
    if (end == false)
        return;

    MessageReceiver rcv(I2CInterface::i2cReadBuffer + 7, I2CInterface::i2cReadBufferIdx - (7 + 5));
    rcv.processMessage();

    I2CInterface::clearReadBuffer();
}

MessageInterface::MessageReceiver::MessageReceiver(const uint8_t *msg, size_t msgSize)
    : _msgSize(msgSize)
{
    memcpy(_message, msg, msgSize);
}

MessageInterface::MessageReceiver::~MessageReceiver()
{
}

void MessageInterface::MessageReceiver::processMessage()
{
    if (verityCheckSum(_message, _msgSize) == false)
    {
        Serial.println("CheckSum error");
        return;
    }

    const MsgCommonHeader *msgHeader = (const MsgCommonHeader *)_message;
    const uint8_t msgId = msgHeader->msgId; // 메시지 ID
    const uint32_t seqId = ntohl(msgHeader->seqId);
    const uint8_t numOfData = msgHeader->numOfData; // 데이터의 수

    char logStr[128] = {0,};
    snprintf(logStr, sizeof(logStr), "[%s:%d] Recv. MsgId:0x%X SeqId:%d\n", __FUNCTION__, __LINE__, msgId, seqId);
    Serial.print(logStr);

    switch (msgId)
    {
    case MsgId::RequestGet:
    {
        const RequestGetData *dataArr = (const RequestGetData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeRequestGetCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::RequestSet:
    {
        const RequestSetData *dataArr = (const RequestSetData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeRequestSetCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::Subscribe:
    {
        const SubscribeData *dataArr = (const SubscribeData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeSubscribeCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::Unsubscribe:
    {
        const UnsubscribeData *dataArr = (const UnsubscribeData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeUnsubscribeCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::Notification:
    {
        const NotificationData *dataArr = (const NotificationData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeNotificationCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::ResponseGet:
    {
        const ResponseGetData *dataArr = (const ResponseGetData *)(_message + sizeof(MsgCommonHeader));
        CallBack::invokeResponseGetCallBack(seqId, dataArr, numOfData);
    }
    break;
    case MsgId::Acknowledge:
    {
        CallBack::invokeAcknowledgeCallBack(seqId, msgId);
    }
    break;
    default:
        return;
    }
}
