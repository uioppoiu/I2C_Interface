#ifndef __MESSAGE_INTERFACE_UTIL_H__
#define __MESSAGE_INTERFACE_UTIL_H__

#include "MessageInterfaceDefinition.h"

namespace MessageInterface
{
    uint8_t getCheckSum(const uint8_t *str, size_t strSize);
    bool verityCheckSum(const uint8_t *str, size_t strSize);

    void writeEndian(ResponseGetData *data); // hton
    void readEndian(ResponseGetData *data);  // ntoh
} // namespace MessageInterface

#endif