#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include "src/I2CInterface/I2CInterface.h"

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
    I2CInterface::MessageSender notiMsg(I2CInterface::MsgId::Notification);
    notiMsg.setSeqId(v++);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorWaterTemperature, 1 + v);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorRoomTemperature, 2 + v);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorCO2, 3 + v);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorHumidity, 4 + v);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorConductivity, 5 + v);
    notiMsg.appendNotificationData(I2CInterface::DataType::SensorPH, 6 + v);
    notiMsg.sendMessage();
}