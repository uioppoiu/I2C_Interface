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