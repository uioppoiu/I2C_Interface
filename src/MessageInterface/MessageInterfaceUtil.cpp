#include "MessageInterfaceUtil.h"
#include "Endian.h"

uint8_t MessageInterface::getCheckSum(const uint8_t *str, size_t strSize)
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

bool MessageInterface::verityCheckSum(const uint8_t *str, size_t strSize)
{
    uint8_t sum = 0;
    for (size_t idx = 0; idx < strSize; idx++)
    {
        sum = sum + str[idx];
    }

    // cout << "Verity : " << (sum == 0) << endl;
    return (sum == 0);
}

void MessageInterface::writeEndian(ResponseGetData *data)
{
    data->value = htonl(data->value);
}

void MessageInterface::readEndian(ResponseGetData *data)
{
    data->value = ntohl(data->value);
}
