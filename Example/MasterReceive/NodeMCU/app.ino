#include <stdio.h>
#include <Wire.h>
#include "src/I2CInterface/I2CInterface.h"

#define LOG_STRING_MAX_SIZE (64)
char logStr[LOG_STRING_MAX_SIZE] = {
    0,
};

const char *WIFI_ID = "ANH_2.4G";
const char *WIFI_PASS = "12345678";
const char *URL_BASE = "http://anhands.synology.me/insert.php";

void setup()
{
    // Serial
    Serial.begin(115200);
    Serial.println("\nSerial Initialized");

    // LED
    pinMode(LED_BUILTIN, OUTPUT);
    snprintf(logStr, sizeof(logStr), "LED Initialized. Pin:%d", LED_BUILTIN);
    Serial.println(logStr);

    // I2C : Master
    I2CInterface::I2C_Master::init();

    // Callback
    I2CInterface::CallBack::registerNotificationCallBack(onNotification);
}

int loopCount_200ms = 0;
int loopCount_400ms = 0;
bool ledState = 0;
void loop()
{
    if (loopCount_400ms == 0)
    {
        digitalWrite(LED_BUILTIN, ledState);
        ledState = !ledState;
    }

    if (loopCount_200ms == 0)
    {
        I2CInterface::I2C_Master::readSlaveBuffer();
    }

    if (loopCount_200ms == 1)
    {
        I2CInterface::MessageReceiver::listen();
    }

    loopCount_200ms = (loopCount_200ms + 1) % 20;
    loopCount_400ms = (loopCount_200ms + 1) % 40;
    delay(10);
}

void onNotification(uint32_t seqId, const I2CInterface::NotificationData *dataArr, size_t arrSize)
{
    Serial.println(__FUNCTION__);
    for(size_t arrIdx = 0 ; arrIdx < arrSize ; arrIdx++)
    {
        const I2CInterface::ResponseGetData& data = dataArr[arrIdx];
        Serial.print("SeqId:");
        Serial.print(seqId);
        Serial.print(" Type:");
        Serial.print((uint32_t)data.type);
        Serial.print(" Value:");
        Serial.println(data.value);
    }

    enum
    {
        IDX_HUMIDITY = 0,
        IDX_CO2 = 1,
        IDX_TEMP_ROOM = 2,
        IDX_CONDUCTIVITY = 3,
        IDX_PH = 4,
        IDX_TEMP_WATER = 5,
    };
    const size_t NumOfSensor = 6;
    const char *sensorTypeStr[NumOfSensor] = {"humidity", "co2", "temperature", "cond", "ph", "watertemp"};
    uint32_t sensorVal[NumOfSensor] = {
        0,
    };

    for (size_t arrIdx = 0; arrIdx < arrSize; arrIdx++)
    {
        const I2CInterface::ResponseGetData &msg = dataArr[arrIdx];

        switch (msg.type)
        {
        case I2CInterface::DataType::SensorWaterTemperature:
            sensorVal[IDX_TEMP_WATER] = msg.value;
            break;
        case I2CInterface::DataType::SensorRoomTemperature:
            sensorVal[IDX_TEMP_ROOM] = msg.value;
            break;
        case I2CInterface::DataType::SensorCO2:
            sensorVal[IDX_CO2] = msg.value;
            break;
        case I2CInterface::DataType::SensorHumidity:
            sensorVal[IDX_HUMIDITY] = msg.value;
            break;
        case I2CInterface::DataType::SensorConductivity:
            sensorVal[IDX_CONDUCTIVITY] = msg.value;
            break;
        case I2CInterface::DataType::SensorPH:
            sensorVal[IDX_PH] = msg.value;
            break;
        default:
            break;
        }
    }

    String dbMsg(URL_BASE);
    for (size_t dbIdx = 0; dbIdx < NumOfSensor; dbIdx++)
    {
        dbMsg += ((dbIdx == 0) ? "?" : "&");
        dbMsg += sensorTypeStr[dbIdx];
        dbMsg += "=";
        dbMsg += String(sensorVal[dbIdx]);
    }

    Serial.print("data:");
    Serial.println(dbMsg);

    // WiFiClient client;
    // HTTPClient http;
    // http.begin(client, dbMsg);
    // http.setTimeout(10000);
    // int ret = http.GET();
    // http.end();

    // Serial.printf("Ret:%d. Done\n", ret);
}
