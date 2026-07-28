#pragma once
#include <stdint.h>
#include <stddef.h>

enum class I2CStatus{
    Ok,
    InvalidArgument,
    UnsupportedFrequency,
    TimeOut,
    StartConditionFailed,
    AddressNack,
    DataNack,
    ArbitrationLost,
    BusError,
    ReceiveFailed,
    StopConditionFailed,
    InvalidAddress,
};

I2CStatus I2CInit(uint32_t frequency);
I2CStatus I2CWrite(uint8_t address, const uint8_t* data, size_t length);
I2CStatus I2CRead(uint8_t address, uint8_t* data, size_t length);
I2CStatus I2CWriteRead(uint8_t address, const uint8_t* writeData, size_t writeLength, uint8_t* readData, size_t readLength);