#include <Arduino.h>
#include "I2CInterface.h"
#include "../MessageInterface/MessageInterface.h"

uint8_t I2CInterface::i2cReadBuffer[I2C_INTERFACE_BUFFER_SIZE] = {
    0,
};
size_t I2CInterface::i2cReadBufferIdx = 0;

uint8_t I2CInterface::i2cWriteBuffer[I2C_INTERFACE_BUFFER_SIZE] = {
    0,
};
size_t I2CInterface::i2cWriteBufferIdx = 0;
size_t I2CInterface::i2cWriteBufferSize = 0;

int I2CInterface::I2C_Slave::commandType = 0;

void I2CInterface::I2C_Master::init()
{
    // I2C : Master
    Wire.begin();
    Serial.println("I2C Master Initialized");
}

void I2CInterface::I2C_Master::readSlaveBuffer(int slaveAddr)
{
    Wire.beginTransmission(slaveAddr);
    Wire.write("RequestDataSize");
    int retVal = Wire.endTransmission();
    // snprintf(logStr, sizeof(logStr), "[%s] Request data size (ret:%d)", __FUNCTION__, retVal);
    // Serial.println(logStr);

    size_t dataSize = 0;
    retVal = Wire.requestFrom(slaveAddr, 1); // Size만 가져온다 (1byte data)
    if (retVal >= 0)
    {
        while (Wire.available())
        {
            dataSize = Wire.read();
        }
        // Serial.printf("[%s:%d] Read recv buffer size:%d\n", __FUNCTION__, __LINE__, dataSize);
    }

    if (dataSize > 0)
    {
        // Serial.printf("[%s:%d] Read recv buffer\n", __FUNCTION__, __LINE__);

        Wire.beginTransmission(slaveAddr);
        Wire.write("RequestData");
        retVal = Wire.endTransmission();

        int numOfFragment = (dataSize + I2C_INTERNAL_BUFFER_SIZE) / I2C_INTERNAL_BUFFER_SIZE;
        for (int fragmentIdx = 0; fragmentIdx < numOfFragment; fragmentIdx++)
        {
            size_t fragmentSize = ((fragmentIdx + 1) == numOfFragment) ? (dataSize % I2C_INTERNAL_BUFFER_SIZE) : I2C_INTERNAL_BUFFER_SIZE;

            retVal = Wire.requestFrom(slaveAddr, fragmentSize);
            if (retVal >= 0)
            {
                while (Wire.available())
                {
                    i2cReadBuffer[i2cReadBufferIdx++] = (uint8_t)Wire.read();

                    // Serial.printf("%d(%d)", i2cReadBufferIdx - 1, (int)i2cReadBuffer[i2cReadBufferIdx - 1]);
                    // Serial.print(' ');

                    if (i2cReadBufferIdx == I2C_INTERFACE_BUFFER_SIZE)
                    {
                        // [Caution] Buffer size를 128로 잡았지만 Size가 128가 되면 Buffer를 읽기 전에 바로 비우기 때문에 실질적으로 127으로 동작함.
                        memset(i2cReadBuffer, 0x00, sizeof(i2cReadBuffer));
                        i2cReadBufferIdx = 0;
                    }
                }
            }
            // Serial.println();
        }
    }
}

void I2CInterface::I2C_Master::writeSlaveBuffer(int slaveAddr)
{
    // char logStr[64] = {
    //     0,
    // };

    int numOfFragment = (i2cWriteBufferSize + I2C_INTERNAL_BUFFER_SIZE) / I2C_INTERNAL_BUFFER_SIZE;
    for (int fragmentIdx = 0; fragmentIdx < numOfFragment; fragmentIdx++)
    {
        Wire.beginTransmission(slaveAddr);
        size_t fragmentSize = ((fragmentIdx + 1) == numOfFragment) ? (i2cWriteBufferSize % I2C_INTERNAL_BUFFER_SIZE) : I2C_INTERNAL_BUFFER_SIZE;
        int retVal = Wire.write(i2cWriteBuffer + (I2C_INTERNAL_BUFFER_SIZE * fragmentIdx), fragmentSize);
        // snprintf(logStr, sizeof(logStr), "[%s:%d] Send Buffer(%d)", __FUNCTION__, __LINE__, retVal);
        // Serial.println(logStr);

        // for (int i = 0; i < fragmentSize; i++)
        // {
        //     Serial.print((int)i2cWriteBuffer[I2C_INTERNAL_BUFFER_SIZE * fragmentIdx + i]);
        //     Serial.print(' ');
        // }
        // Serial.println();

        Wire.endTransmission();
    }

    clearWriteBuffer();
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

    while (Wire.available()) // slave may send less than requested
    {
        int byte = Wire.read();
        i2cReadBuffer[i2cReadBufferIdx++] = (char)byte;
    }

    if (memcmp(i2cReadBuffer, "RequestDataSize", sizeof("RequestDataSize")) == 0)
    {
        // char logStr[64] = {
        //     0,
        // };
        // snprintf(logStr, sizeof(logStr), "[%s] Request Size", __FUNCTION__);
        // Serial.println(logStr);

        commandType = 0; // Size
        clearReadBuffer();
    }
    else if (memcmp(i2cReadBuffer, "RequestData", sizeof("RequestData")) == 0)
    {
        // char logStr[64] = {
        //     0,
        // };
        // snprintf(logStr, sizeof(logStr), "[%s] Request Data", __FUNCTION__);
        // Serial.println(logStr);

        commandType = 1; // Data
        clearReadBuffer();
    }
    else
    {
        MessageInterface::MessageReceiver::listen();
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
        // snprintf(logStr, sizeof(logStr), "[%s:%d] Send Buffer size(%d)", __FUNCTION__, __LINE__, i2cWriteBufferSize);
        // Serial.println(logStr);
        numOfFragment = (i2cWriteBufferSize + I2C_INTERNAL_BUFFER_SIZE) / I2C_INTERNAL_BUFFER_SIZE;
        // snprintf(logStr, sizeof(logStr), "[%s:%d] NumOfFragment:%d", __FUNCTION__, __LINE__ numOfFragment);
        // Serial.println(logStr);
    }
    break;
    case 1: // Data
    {
        size_t fragmentSize = ((fragmentIdx + 1) == numOfFragment) ? (i2cWriteBufferSize % I2C_INTERNAL_BUFFER_SIZE) : I2C_INTERNAL_BUFFER_SIZE;
        int retVal = Wire.write(i2cWriteBuffer + (I2C_INTERNAL_BUFFER_SIZE * fragmentIdx), fragmentSize);
        // snprintf(logStr, sizeof(logStr), "[%s:%d] Send Buffer(%d)", __FUNCTION__, __LINE__ retVal);
        // Serial.println(logStr);

        // for (int i = 0; i < fragmentSize; i++)
        // {
        //     Serial.print((int)i2cWriteBuffer[I2C_INTERNAL_BUFFER_SIZE * fragmentIdx + i]);
        //     Serial.print(' ');
        // }
        // Serial.println();

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
    for (int i = 0; i < I2C_INTERFACE_BUFFER_SIZE; i++)
    {
        i2cWriteBuffer[i] = 0;
    }
    i2cWriteBufferIdx = 0;
    i2cWriteBufferSize = 0;
}

void I2CInterface::clearReadBuffer()
{
    for (int i = 0; i < I2C_INTERFACE_BUFFER_SIZE; i++)
    {
        i2cReadBuffer[i] = 0;
    }
    i2cReadBufferIdx = 0;
}