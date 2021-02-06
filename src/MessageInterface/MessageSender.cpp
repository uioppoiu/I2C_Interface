#include <Arduino.h>
#include "MessageSender.h"
#include "../I2CInterface/I2CInterface.h"
#include "Endian.h"

MessageInterface::MessageSender::MessageSender(const unsigned char msgId)
    : _header((MsgCommonHeader *)_messageBuffer)
{
    memset(_messageBuffer, 0x00, sizeof(_messageBuffer));
    _header->msgId = msgId;
    _header->msgSize = sizeof(MsgCommonHeader);
}

MessageInterface::MessageSender::~MessageSender()
{
}

void MessageInterface::MessageSender::setSeqId(uint32_t seqId)
{
    _header->seqId = htonl(seqId);
}

void MessageInterface::MessageSender::sendMessage()
{
    noInterrupts();

    uint8_t checkSum = getCheckSum(_messageBuffer, _header->msgSize);
    _messageBuffer[_header->msgSize] = checkSum;
    _header->msgSize += 1;

    I2CInterface::clearWriteBuffer();
    I2CInterface::writeWriteBuffer((uint8_t*)"<BEGIN>", 7);
    I2CInterface::writeWriteBuffer(_messageBuffer, _header->msgSize);
    I2CInterface::writeWriteBuffer((uint8_t*)"<END>", 5);

    // char logStr[64] = {
    //     0,
    // };
    // snprintf(logStr, sizeof(logStr), "[%s:%d] Write Send Message. Size(%d)", __FUNCTION__, __LINE__, _header->msgSize);
    // Serial.println(logStr);

    interrupts();
}

void MessageInterface::MessageSender::appendRequestGetDataCommon(unsigned char dataType)
{
    RequestGetData data;
    memset(&data, 0x00, sizeof(RequestGetData));

    data.type = dataType;

    appendData(data);
}

void MessageInterface::MessageSender::appendRequestGetData(unsigned char dataType)
{
    if (_header->msgId != MsgId::RequestGet)
        return;

    appendRequestGetDataCommon(dataType);
}

void MessageInterface::MessageSender::appendSubscribeData(unsigned char dataType)
{
    if (_header->msgId != MsgId::Subscribe)
        return;

    appendRequestGetDataCommon(dataType);
}

void MessageInterface::MessageSender::appendUnsubscribeData(unsigned char dataType)
{
    if (_header->msgId != MsgId::Unsubscribe)
        return;

    appendRequestGetDataCommon(dataType);
}

void MessageInterface::MessageSender::appendResponseGetDataCommon(unsigned char dataType, uint32_t value)
{
    ResponseGetData data;
    memset(&data, 0x00, sizeof(ResponseGetData));

    data.type = dataType;
    data.value = value;
    writeEndian(&data);

    appendData(data);
}

void MessageInterface::MessageSender::appendResponseGetData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::ResponseGet)
        return;

    appendResponseGetDataCommon(dataType, value);
}

void MessageInterface::MessageSender::appendNotificationData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::Notification)
        return;

    appendResponseGetDataCommon(dataType, value);
}

void MessageInterface::MessageSender::appendRequestSetData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::RequestSet)
        return;

    appendResponseGetDataCommon(dataType, value);
}
