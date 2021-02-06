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
}

int loopCount = 0;
bool ledState = 0;
void loop()
{
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;

    sendTestMessage();

    loopCount = (++loopCount) % 1000;
    delay(3000);
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