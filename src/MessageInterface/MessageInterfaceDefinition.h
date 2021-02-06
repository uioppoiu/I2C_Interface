#ifndef __MESSAGE_INTERFACE_DEFINITION_H__
#define __MESSAGE_INTERFACE_DEFINITION_H__

#include <Arduino.h>

namespace MessageInterface
{
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
};

#endif