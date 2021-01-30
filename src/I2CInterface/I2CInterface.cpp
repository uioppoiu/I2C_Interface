#include <Arduino.h>
#include "I2CInterface.h"
#include "Endian.h"

uint8_t I2CInterface::i2cReadBuffer[I2C_INTERFACE_BUFFER_SIZE] = {
    0,
};
size_t I2CInterface::i2cReadBufferIdx = 0;

uint8_t I2CInterface::i2cWriteBuffer[I2C_INTERFACE_BUFFER_SIZE] = {
    0,
};
size_t I2CInterface::i2cWriteBufferIdx = 0;
size_t I2CInterface::i2cWriteBufferSize = 0;

I2CInterface::CallBack::CallBackRequestGet I2CInterface::CallBack::_callBackRequestGet = NULL;
I2CInterface::CallBack::CallBackResponseGet I2CInterface::CallBack::_callBackResponseGet = NULL;
I2CInterface::CallBack::CallBackRequestSet I2CInterface::CallBack::_callBackRequestSet = NULL;
I2CInterface::CallBack::CallBackSubscribe I2CInterface::CallBack::_callBackSubscribe = NULL;
I2CInterface::CallBack::CallBackUnsubscribe I2CInterface::CallBack::_callBackUnsubscribe = NULL;
I2CInterface::CallBack::CallBackNotification I2CInterface::CallBack::_callBackNotification = NULL;
I2CInterface::CallBack::CallBackAcknowledge I2CInterface::CallBack::_callBackAcknowledge = NULL;

int I2CInterface::I2C_Slave::commandType = 0;

uint8_t I2CInterface::getCheckSum(const uint8_t *str, size_t strSize)
{
    uint8_t sum = 0;
    for (size_t idx = 0; idx < strSize; idx++)
    {
        sum = sum + str[idx];
    }

    uint8_t checkSum = (sum ^ 0xFF);

    // cout << "CheckSum : " << (int)checkSum << endl;
    return checkSum;
}

bool I2CInterface::verityCheckSum(const uint8_t *str, size_t strSize)
{
    uint8_t sum = 0;
    for (size_t idx = 0; idx < strSize; idx++)
    {
        sum = sum + str[idx];
    }

    // cout << "Verity : " << (sum == 0) << endl;
    return (sum == 0);
}

void I2CInterface::writeEndian(ResponseGetData *data)
{
    data->value = htonl(data->value);
}

void I2CInterface::readEndian(ResponseGetData *data)
{
    data->value = ntohl(data->value);
}

void I2CInterface::CallBack::registerRequestGetCallBack(const CallBackRequestGet func)
{
    _callBackRequestGet = func;
}

void I2CInterface::CallBack::invokeRequestGetCallBack(uint32_t seqId, const RequestGetData *dataArr, size_t arrSize)
{
    if (_callBackRequestGet == NULL)
        return;
    _callBackRequestGet(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerResponseGetCallBack(const CallBackResponseGet func)
{
    _callBackResponseGet = func;
}

void I2CInterface::CallBack::invokeResponseGetCallBack(uint32_t seqId, const ResponseGetData *dataArr, size_t arrSize)
{
    if (_callBackResponseGet == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        I2CInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackResponseGet(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerNotificationCallBack(const CallBackNotification func)
{
    _callBackNotification = func;
}

void I2CInterface::CallBack::invokeNotificationCallBack(uint32_t seqId, const NotificationData *dataArr, size_t arrSize)
{
    if (_callBackNotification == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        I2CInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackNotification(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerSubscribeCallBack(const CallBackSubscribe func)
{
    _callBackSubscribe = func;
}

void I2CInterface::CallBack::invokeSubscribeCallBack(uint32_t seqId, const SubscribeData *dataArr, size_t arrSize)
{
    if (_callBackSubscribe == NULL)
        return;
    _callBackSubscribe(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerUnsubscribeCallBack(const CallBackUnsubscribe func)
{
    _callBackUnsubscribe = func;
}

void I2CInterface::CallBack::invokeUnsubscribeCallBack(uint32_t seqId, const UnsubscribeData *dataArr, size_t arrSize)
{
    if (_callBackUnsubscribe == NULL)
        return;
    _callBackUnsubscribe(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerRequestSetCallBack(const CallBackRequestSet func)
{
    _callBackRequestSet = func;
}

void I2CInterface::CallBack::invokeRequestSetCallBack(uint32_t seqId, const RequestSetData *dataArr, size_t arrSize)
{
    if (_callBackRequestSet == NULL)
        return;

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        I2CInterface::readEndian((ResponseGetData *)(dataArr + arrIdx));
    }

    _callBackRequestSet(seqId, dataArr, arrSize);
}

void I2CInterface::CallBack::registerAcknowledgeCallBack(const CallBackAcknowledge func)
{
    _callBackAcknowledge = func;
}

void I2CInterface::CallBack::invokeAcknowledgeCallBack(uint32_t seqId, unsigned char msgId)
{
    if (_callBackAcknowledge == NULL)
        return;
    _callBackAcknowledge(seqId, msgId);
}

void I2CInterface::MessageReceiver::listen()
{
    bool begin = false;
    if (i2cReadBufferIdx >= 7)
    {
        if (memcmp(i2cReadBuffer, "<BEGIN>", 7) == 0)
        {
            begin = true;
        }
    }

    bool end = false;
    if (i2cReadBufferIdx >= 5)
    {
        if (memcmp(i2cReadBuffer + i2cReadBufferIdx - 5, "<END>", 5) == 0)
        {
            end = true;
        }
    }

    if(begin && end)
    {
        MessageReceiver rcv(i2cReadBuffer + 7, i2cReadBufferIdx - (7 + 5));
        rcv.processMessage();

        memset(i2cReadBuffer, 0x00, sizeof(i2cReadBuffer));
        i2cReadBufferIdx = 0;
    }
}

I2CInterface::MessageReceiver::MessageReceiver(const uint8_t *msg, size_t msgSize)
    : _msgSize(msgSize)
{
    memcpy(_message, msg, msgSize);
}

I2CInterface::MessageReceiver::~MessageReceiver()
{
}

void I2CInterface::MessageReceiver::processMessage()
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

    Serial.print("MsgId:0x");
    Serial.print(msgId,16);
    Serial.print(" SeqId:");
    Serial.println(seqId);

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

I2CInterface::MessageSender::MessageSender(const unsigned char msgId)
    : _header((MsgCommonHeader *)_messageBuffer)
{
    memset(_messageBuffer, 0x00, sizeof(_messageBuffer));
    _header->msgId = msgId;
    _header->msgSize = sizeof(MsgCommonHeader);
}

I2CInterface::MessageSender::~MessageSender()
{
}

void I2CInterface::MessageSender::setSeqId(uint32_t seqId)
{
    _header->seqId = htonl(seqId);
}

void I2CInterface::MessageSender::sendMessage()
{
    uint8_t checkSum = getCheckSum(_messageBuffer, _header->msgSize);
    _messageBuffer[_header->msgSize] = checkSum;
    _header->msgSize += 1;

    writeWriteBuffer((uint8_t*)"<BEGIN>", 7);
    writeWriteBuffer(_messageBuffer, _header->msgSize);
    writeWriteBuffer((uint8_t*)"<END>", 5);
}

void I2CInterface::MessageSender::appendRequestGetDataCommon(unsigned char dataType)
{
    RequestGetData data;
    memset(&data, 0x00, sizeof(RequestGetData));

    data.type = dataType;

    appendData(data);
}

void I2CInterface::MessageSender::appendRequestGetData(unsigned char dataType)
{
    if (_header->msgId != MsgId::RequestGet)
        return;

    appendRequestGetDataCommon(dataType);
}

void I2CInterface::MessageSender::appendSubscribeData(unsigned char dataType)
{
    if (_header->msgId != MsgId::Subscribe)
        return;

    appendRequestGetDataCommon(dataType);
}

void I2CInterface::MessageSender::appendUnsubscribeData(unsigned char dataType)
{
    if (_header->msgId != MsgId::Unsubscribe)
        return;

    appendRequestGetDataCommon(dataType);
}

void I2CInterface::MessageSender::appendResponseGetDataCommon(unsigned char dataType, uint32_t value)
{
    ResponseGetData data;
    memset(&data, 0x00, sizeof(ResponseGetData));

    data.type = dataType;
    data.value = htonl(value);

    appendData(data);
}

void I2CInterface::MessageSender::appendResponseGetData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::ResponseGet)
        return;

    appendResponseGetDataCommon(dataType, value);
}

void I2CInterface::MessageSender::appendNotificationData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::Notification)
        return;

    appendResponseGetDataCommon(dataType, value);
}

void I2CInterface::MessageSender::appendRequestSetData(unsigned char dataType, uint32_t value)
{
    if (_header->msgId != MsgId::RequestSet)
        return;

    appendResponseGetDataCommon(dataType, value);
}

void I2CInterface::I2C_Master::init()
{
    // I2C : Master
    Wire.begin();
    Serial.println("I2C Master Initialized");
}

void I2CInterface::I2C_Master::readSlaveBuffer()
{
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write("RequestDataSize");
    int retVal = Wire.endTransmission();
    // snprintf(logStr, sizeof(logStr), "[%s] Request data size (ret:%d)", __FUNCTION__, retVal);
    // Serial.println(logStr);

    size_t dataSize = 0;
    retVal = Wire.requestFrom(I2C_SLAVE_ADDRESS, 1); // Size만 가져온다 (1byte data)
    if (retVal >= 0)
    {
        while (Wire.available())
        {
            dataSize = Wire.read();
        }
    }

    if (dataSize > 0)
    {
        Wire.beginTransmission(I2C_SLAVE_ADDRESS);
        Wire.write("RequestData");
        retVal = Wire.endTransmission();

        int numOfFragment = (dataSize + I2C_BUFFER_SIZE) / I2C_BUFFER_SIZE;
        for (int fragmentIdx = 0; fragmentIdx < numOfFragment; fragmentIdx++)
        {
            size_t fragmentSize = ((fragmentIdx + 1) == numOfFragment) ? (dataSize % I2C_BUFFER_SIZE) : I2C_BUFFER_SIZE;

            retVal = Wire.requestFrom(I2C_SLAVE_ADDRESS, fragmentSize);
            if (retVal >= 0)
            {
                while (Wire.available())
                {
                    i2cReadBuffer[i2cReadBufferIdx++] = (uint8_t)Wire.read();
                    if (i2cReadBufferIdx == I2C_INTERFACE_BUFFER_SIZE)
                    {
                        // [Caution] Buffer size를 128로 잡았지만 Size가 128가 되면 Buffer를 읽기 전에 바로 비우기 때문에 실질적으로 127으로 동작함.
                        memset(i2cReadBuffer, 0x00, sizeof(i2cReadBuffer));
                        i2cReadBufferIdx = 0;
                    }
                }
            }
        }
    }
}

void I2CInterface::I2C_Slave::init()
{
    // I2C : Slave
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    Wire.begin(I2C_SLAVE_ADDRESS);

    char logStr[64] = {
        0,
    };
    snprintf(logStr, sizeof(logStr), "I2C Slave Initialized. Address:%d", I2C_SLAVE_ADDRESS);
    Serial.println(logStr);
}

void I2CInterface::I2C_Slave::receiveEvent(int numOfBytes)
{
    // snprintf(logStr, sizeof(logStr), "[%s] Data arrives. Size:%d", __FUNCTION__, numOfBytes);
    // Serial.println(logStr);

    char reqCommand[I2C_BUFFER_SIZE] = {
        0,
    };
    size_t reqCommandIdx = 0;
    while (Wire.available()) // slave may send less than requested
    {
        int byte = Wire.read();
        reqCommand[reqCommandIdx++] = (char)byte;
    }

    if (memcmp(reqCommand, "RequestDataSize", sizeof("RequestDataSize")) == 0)
    {
        // char logStr[64] = {
        //     0,
        // };
        // snprintf(logStr, sizeof(logStr), "[%s] Request Size", __FUNCTION__);
        // Serial.println(logStr);

        commandType = 0; // Size
    }
    else if (memcmp(reqCommand, "RequestData", sizeof("RequestData")) == 0)
    {
        // char logStr[64] = {
        //     0,
        // };
        // snprintf(logStr, sizeof(logStr), "[%s] Request Data", __FUNCTION__);
        // Serial.println(logStr);

        commandType = 1; // Data
    }
}

void I2CInterface::I2C_Slave::requestEvent()
{
    char logStr[64] = {
        0,
    };

    static int numOfFragment = 0;
    static int fragmentIdx = 0;
    switch (commandType)
    {
    case 0: // Size
    {
        int retVal = Wire.write((uint8_t)i2cWriteBufferSize);
        snprintf(logStr, sizeof(logStr), "[%s] Send Buffer size(%d)", __FUNCTION__, i2cWriteBufferSize);
        Serial.println(logStr);
        numOfFragment = (i2cWriteBufferSize + I2C_BUFFER_SIZE) / I2C_BUFFER_SIZE;
        snprintf(logStr, sizeof(logStr), "[%s] NumOfFragment:%d", __FUNCTION__, numOfFragment);
        Serial.println(logStr);
    }
    break;
    case 1: // Data
    {
        size_t fragmentSize = ((fragmentIdx + 1) == numOfFragment) ? (i2cWriteBufferSize % I2C_BUFFER_SIZE) : I2C_BUFFER_SIZE;
        int retVal = Wire.write(i2cWriteBuffer + (I2C_BUFFER_SIZE * fragmentIdx), fragmentSize);
        snprintf(logStr, sizeof(logStr), "[%s] Send Buffer(%d)", __FUNCTION__, retVal);
        Serial.println(logStr);

        for (int i = 0; i < fragmentSize; i++)
        {
            Serial.print((int)i2cWriteBuffer[I2C_BUFFER_SIZE * fragmentIdx + i]);
            Serial.print(' ');
        }
        Serial.println();

        fragmentIdx++;
        if (fragmentIdx == numOfFragment)
        {
            fragmentIdx = 0;
            numOfFragment = 0;
            clearWriteBuffer();
        }
    }
    break;
    }
}

void I2CInterface::writeWriteBuffer(uint8_t *buf, size_t size)
{
    for (int bufIdx = 0; bufIdx < size; bufIdx++)
    {
        i2cWriteBuffer[i2cWriteBufferIdx++] = buf[bufIdx];
        i2cWriteBufferSize = i2cWriteBufferIdx;
        i2cWriteBufferIdx = i2cWriteBufferIdx % I2C_INTERFACE_BUFFER_SIZE;
    }
}

void I2CInterface::writeWriteBuffer(uint8_t byte)
{
    writeWriteBuffer(&byte, 1);
}

void I2CInterface::clearWriteBuffer()
{
    for (int i = 0; i < I2C_BUFFER_SIZE; i++)
    {
        i2cWriteBuffer[i] = 0;
    }
    i2cWriteBufferIdx = 0;
    i2cWriteBufferSize = 0;
}