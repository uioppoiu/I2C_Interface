#ifndef _I2C_INTERFACE_H_
#define _I2C_INTERFACE_H_

#include <Wire.h>
#include <stdio.h>
#include <string.h>

#define I2C_SLAVE_ADDRESS (10)
#define I2C_BUFFER_SIZE (32)
#define I2C_INTERFACE_BUFFER_SIZE (128)

class I2CInterface
{
public:
    typedef struct
    {
        uint8_t msgId;      // 메시지 ID
        uint8_t msgSize;    // 헤더를 포함한 메시지 크기
        uint8_t numOfData;  // 데이터의 수
        uint8_t sizeOfData; // 각 데이터의 사이즈
        uint32_t seqId;
    } MsgCommonHeader;

    typedef struct
    {
        uint8_t type;
        uint8_t reserved[3];
    } RequestGetData, SubscribeData, UnsubscribeData;

    typedef struct
    {
        uint8_t type;
        uint8_t reserved[3];
        uint32_t value;
    } ResponseGetData, RequestSetData, NotificationData;

    class MsgId
    {
    public:
        static const uint8_t RequestGet = 0x10;
        static const uint8_t RequestSet = 0x20;
        static const uint8_t Subscribe = 0x30;
        static const uint8_t Unsubscribe = 0x40;
        static const uint8_t Notification = 0x50;
        static const uint8_t ResponseGet = 0x60;
        static const uint8_t Acknowledge = 0x70;
    };

    class DataType
    {
    public:
        static const uint8_t SensorAll = 0x0F;
        static const uint8_t SensorRoomTemperature = 0x00;
        static const uint8_t SensorCO2 = 0x01;
        static const uint8_t SensorHumidity = 0x02;
        static const uint8_t SensorConductivity = 0x03;
        static const uint8_t SensorPH = 0x04;
        static const uint8_t SensorWaterTemperature = 0x05;
        static const uint8_t ControlAll = 0x1F;
        static const uint8_t Control0 = 0x10;
        static const uint8_t Control1 = 0x11;
        static const uint8_t Control2 = 0x12;
        static const uint8_t Control3 = 0x13;
        static const uint8_t Control4 = 0x14;
        static const uint8_t DateTime = 0x20;
    };

    static uint8_t getCheckSum(const uint8_t *str, size_t strSize);
    static bool verityCheckSum(const uint8_t *str, size_t strSize);

    static void writeEndian(ResponseGetData *data); // hton
    static void readEndian(ResponseGetData *data);  // ntoh

    class CallBack
    {
    public:
        // Callback
        typedef void (*CallBackRequestGet)(uint32_t /*seqId*/, const RequestGetData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackResponseGet)(uint32_t /*seqId*/, const ResponseGetData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackNotification)(uint32_t /*seqId*/, const NotificationData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackSubscribe)(uint32_t /*seqId*/, const SubscribeData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackUnsubscribe)(uint32_t /*seqId*/, const UnsubscribeData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackRequestSet)(uint32_t /*seqId*/, const RequestSetData * /*dataArr*/, size_t /*arrSize*/);
        typedef void (*CallBackAcknowledge)(uint32_t /*seqId*/, unsigned char /*msgId*/);
        static void registerRequestGetCallBack(const CallBackRequestGet func);
        static void invokeRequestGetCallBack(uint32_t seqId, const RequestGetData *dataArr, size_t arrSize);
        static void registerResponseGetCallBack(const CallBackResponseGet func);
        static void invokeResponseGetCallBack(uint32_t seqId, const ResponseGetData *dataArr, size_t arrSize);
        static void registerNotificationCallBack(const CallBackNotification func);
        static void invokeNotificationCallBack(uint32_t seqId, const NotificationData *dataArr, size_t arrSize);
        static void registerSubscribeCallBack(const CallBackSubscribe func);
        static void invokeSubscribeCallBack(uint32_t seqId, const SubscribeData *dataArr, size_t arrSize);
        static void registerUnsubscribeCallBack(const CallBackUnsubscribe func);
        static void invokeUnsubscribeCallBack(uint32_t seqId, const UnsubscribeData *dataArr, size_t arrSize);
        static void registerRequestSetCallBack(const CallBackRequestSet func);
        static void invokeRequestSetCallBack(uint32_t seqId, const RequestSetData *dataArr, size_t arrSize);
        static void registerAcknowledgeCallBack(const CallBackAcknowledge func);
        static void invokeAcknowledgeCallBack(uint32_t seqId, unsigned char msgId);

    private:
        // Callback
        static CallBackRequestGet _callBackRequestGet;
        static CallBackResponseGet _callBackResponseGet;
        static CallBackNotification _callBackNotification;
        static CallBackSubscribe _callBackSubscribe;
        static CallBackUnsubscribe _callBackUnsubscribe;
        static CallBackRequestSet _callBackRequestSet;
        static CallBackAcknowledge _callBackAcknowledge;
    };

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

    class MessageSender
    {
    public:
        MessageSender(const unsigned char msgId);
        ~MessageSender();

        void setSeqId(uint32_t seqId);

        template <typename T>
        void appendData(const T &data)
        {
            if (_header->sizeOfData == 0)
            {
                _header->sizeOfData = sizeof(T);
            }

            if (_header->sizeOfData != sizeof(T))
            {
                // Data type을 섞어서 append할 수 없다.
                return;
            }

            T *dataPtr = (T *)(_messageBuffer + sizeof(MsgCommonHeader));
            memcpy(dataPtr + _header->numOfData, &data, sizeof(T));

            _header->msgSize += sizeof(T);
            _header->numOfData += 1;
        }

        void appendRequestGetDataCommon(unsigned char dataType);
        void appendRequestGetData(unsigned char dataType);
        void appendSubscribeData(unsigned char dataType);
        void appendUnsubscribeData(unsigned char dataType);

        void appendResponseGetDataCommon(unsigned char dataType, uint32_t value);
        void appendResponseGetData(unsigned char dataType, uint32_t value);
        void appendNotificationData(unsigned char dataType, uint32_t value);
        void appendRequestSetData(unsigned char dataType, uint32_t value);

        void sendMessage();

    private:
        uint8_t _messageBuffer[128];
        MsgCommonHeader *_header;
        MessageSender();
    };

    // I2C로 주고받을 data를 담고 있는 Buffer
    static uint8_t i2cReadBuffer[I2C_INTERFACE_BUFFER_SIZE];
    static size_t i2cReadBufferIdx;
    static uint8_t i2cWriteBuffer[I2C_INTERFACE_BUFFER_SIZE];
    static size_t i2cWriteBufferIdx;
    static size_t i2cWriteBufferSize;

    static void writeWriteBuffer(uint8_t *buf, size_t size);
    static void writeWriteBuffer(uint8_t byte);
    static void clearWriteBuffer();

    class I2C_Master
    {
    public:
        // I2C Interface
        static void init();
        static void readSlaveBuffer(); // Slave측 Buffer에서 data를 가져온다.
    };

    class I2C_Slave
    {
    public:
        static void init();
        static void receiveEvent(int numOfBytes);
        static void requestEvent();

    private:
        static int commandType;
    };
};

#endif