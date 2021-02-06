#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include "src/I2CInterface/I2CInterface.h"
#include "src/MessageInterface/MessageInterface.h"

#define LOG_STRING_MAX_SIZE (64)
char logStr[LOG_STRING_MAX_SIZE] = {0,};

void setup()
{
    // Serial
    Serial.begin(115200);
    Serial.println("\nSerial Initialized");

    // LED
    pinMode(LED_BUILTIN, OUTPUT);
    snprintf(logStr, sizeof(logStr), "LED Initialized. Pin:%d", LED_BUILTIN);
    Serial.println(logStr);

    // I2C : Slave
    I2CInterface::I2C_Slave::init();

    // Callback
    MessageInterface::CallBack::registerRequestSetCallBack(onRequestSet);
}

int loopCount_T1 = 0;
int loopCount_T2 = 0;
int loopCount_T3 = 0;
bool ledState = 0;
void loop()
{
    defaultLoop();

    if (loopCount_T1 == 0)
    {
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
    }

    if (loopCount_T2 == 0)
    {
        sendTestMessage();
    }
}

void defaultLoop()
{
    loopCount_T1 = (loopCount_T1 + 1) % 50;
    loopCount_T2 = (loopCount_T2 + 1) % 500;
    loopCount_T3 = (loopCount_T3 + 1) % 20;
    delay(10);
}

void sendTestMessage()
{
    static int v = 0;
    MessageInterface::MessageSender notiMsg(MessageInterface::MsgId::Notification);
    notiMsg.setSeqId(v++);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorWaterTemperature, 1 + v);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorRoomTemperature, 2 + v);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorCO2, 3 + v);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorHumidity, 4 + v);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorConductivity, 5 + v);
    notiMsg.appendNotificationData(MessageInterface::DataType::SensorPH, 6 + v);
    notiMsg.sendMessage();
}


void onRequestSet(uint32_t seqId, const MessageInterface::RequestSetData *dataArr, size_t arrSize)
{
    Serial.println(__FUNCTION__);
    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        const MessageInterface::RequestSetData &msgData = dataArr[arrIdx];
        Serial.print("SeqId:");
        Serial.print(seqId);
        Serial.print(" Type:");
        Serial.print((uint32_t)msgData.type);
        Serial.print(" Value:");
        Serial.println(msgData.value);
    }
}