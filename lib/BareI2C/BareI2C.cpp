#include "BareI2C.h"
#include <avr/io.h>

namespace
{
    constexpr uint16_t kMaxLoopCount = 50000; 
    
    enum class StartType
    {
        Initial,
        Repeated,
    };

    enum class SendAck
    {
        Ack,
        Nack,
    };
    enum class ReadOrWrite
    {
        Read,
        Write,
    };

    I2CStatus WaitForInt()
    {

        uint32_t count = 0;
        while (!(TWCR & (1 << TWINT)))
        {
            count++;
            if (count == kMaxLoopCount)
            {
                return I2CStatus::TimeOut;
            }
        }
        return I2CStatus::Ok;
    }
    I2CStatus SendStart(StartType condition)
    {
        TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT);
        I2CStatus wait = WaitForInt();
        if (wait != I2CStatus::Ok)
        {
            return wait;
        }

        uint8_t checkStatus = TWSR & 0xF8;
        if (condition == StartType::Initial)
        {
            if (checkStatus == 0x08)
            {
                return I2CStatus::Ok;
            }
        }
        else if (condition == StartType::Repeated)
        {
            if (checkStatus == 0x10)
                return I2CStatus::Ok;
        }

        if (checkStatus == 0x38)
        {
            return I2CStatus::ArbitrationLost;
        }
        else if (checkStatus == 0x00)
        {
            return I2CStatus::BusError;
        }
        return I2CStatus::StartConditionFailed;
    }

    I2CStatus SendStop()
    {
        TWCR = (1 << TWEN) | (1 << TWSTO) | (1 << TWINT);
        uint32_t count = 0;
        while (TWCR & (1 << TWSTO))
        {
            count++;
            if (count == kMaxLoopCount)
            {
                return I2CStatus::StopConditionFailed;
            }
        }
        return I2CStatus::Ok;
    }

    I2CStatus SendAddress(uint8_t address, ReadOrWrite type)
    {
        if (type == ReadOrWrite::Read)
        {
            TWDR = (address << 1) | 1;
        }
        else if (type == ReadOrWrite ::Write)
        {
            TWDR = address << 1;
        }
        TWCR = (1 << TWEN) | (1 << TWINT);
        I2CStatus wait = WaitForInt();
        if (wait != I2CStatus::Ok)
        {
            return wait;
        }

        uint8_t checkStatus = TWSR & 0xF8;
        if (type == ReadOrWrite::Write)
        {
            if (checkStatus == 0x18)
            {
                return I2CStatus::Ok;
            }
            else if (checkStatus == 0x20)
            {
                return I2CStatus::AddressNack;
            }
        }
        if (type == ReadOrWrite::Read)
        {
            if (checkStatus == 0x40)
            {
                return I2CStatus::Ok;
            }
            else if (checkStatus == 0x48)
            {
                return I2CStatus::AddressNack;
            }
        }

        if (checkStatus == 0x38)
        {
            return I2CStatus::ArbitrationLost;
        }
        else if (checkStatus == 0x00)
        {
            return I2CStatus::BusError;
        }
        return I2CStatus::BusError;
    }

    I2CStatus WriteByte(uint8_t data)
    {
        TWDR = data;
        TWCR = (1 << TWEN) | (1 << TWINT);
        I2CStatus wait = WaitForInt();
        if (wait != I2CStatus::Ok)
        {
            return wait;
        }
        uint8_t checkStatus = TWSR & 0xF8;
        if (checkStatus == 0x28)
        {
            return I2CStatus::Ok;
        }
        else if (checkStatus == 0x30)
        {
            return I2CStatus::DataNack;
        }
        if (checkStatus == 0x38)
        {
            return I2CStatus::ArbitrationLost;
        }
        else if (checkStatus == 0x00)
        {
            return I2CStatus::BusError;
        }
        return I2CStatus::BusError;
    }

    I2CStatus ReadByte(uint8_t *data, SendAck respond)
    {
        if (respond == SendAck::Ack)
        {
            TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);
        }
        else if (respond == SendAck::Nack)
        {
            TWCR = (1 << TWEN) | (1 << TWINT);
        }

        I2CStatus wait = WaitForInt();
        if (wait != I2CStatus::Ok)
        {
            return wait;
        }

        uint8_t checkStatus = TWSR & 0xF8;
        if (checkStatus == 0x50 && respond == SendAck::Ack)
        {
            *data = TWDR;
            return I2CStatus::Ok;
        }
        else if (checkStatus == 0x58 && respond == SendAck::Nack)
        {
            *data = TWDR;
            return I2CStatus::Ok;
        }
        else if (checkStatus == 0x38)
        {
            return I2CStatus::ArbitrationLost;
        }
        else if (checkStatus == 0x00)
        {
            return I2CStatus::BusError;
        }
        return I2CStatus::ReceiveFailed;
    }
}

I2CStatus I2CInit(uint32_t frequency)
{
    if (frequency > 400000)
    {
        return I2CStatus::UnsupportedFrequency;
    }
    if (frequency == 0)
    {
        return I2CStatus::InvalidArgument;
    }

    for (uint8_t prescaler = 0; prescaler < 4; ++prescaler)
    {
        uint32_t twbr = ((F_CPU / frequency) - 16) / (2 * (1 << (2 * prescaler)));
        if (twbr <= 255)
        {
            TWSR = prescaler;
            TWBR = static_cast<uint8_t>(twbr);
            TWCR = (1 << TWEN);
            return I2CStatus::Ok;
        }
    }
    return I2CStatus::UnsupportedFrequency;
}

I2CStatus I2CWrite(uint8_t address, const uint8_t *data, size_t length)
{
    if (address < 0x08 || address > 0x77)
    {
        return I2CStatus::InvalidAddress;
    }

    if (data == nullptr && length > 0)
    {
        return I2CStatus::InvalidArgument;
    }

    I2CStatus checkStart = SendStart(StartType::Initial);
    if (checkStart != I2CStatus::Ok)
    {
        return checkStart;
    }

    I2CStatus checkAddress = SendAddress(address, ReadOrWrite::Write);
    if (checkAddress == I2CStatus::AddressNack)
    {
        I2CStatus checkStop = SendStop();
        if (checkStop != I2CStatus::Ok)
        {
            return checkStop;
        }
        return checkAddress;
    }
    else if (checkAddress != I2CStatus::Ok)
    {
        return checkAddress;
    }

    for (size_t index = 0; index < length; index++)
    {
        I2CStatus checkWrite = WriteByte(data[index]);
        if (checkWrite == I2CStatus::DataNack)
        {
            I2CStatus checkStop = SendStop();
            if (checkStop != I2CStatus::Ok)
            {
                return checkStop;
            }
            return checkWrite;
        }
        else if (checkWrite != I2CStatus::Ok)
        {
            return checkWrite;
        }
    }

    I2CStatus checkStop = SendStop();
    if (checkStop != I2CStatus::Ok)
    {
        return checkStop;
    }

    return I2CStatus::Ok;
}

I2CStatus I2CRead(uint8_t address, uint8_t *data, size_t length)
{
    if (address < 0x08 | address > 0x77)
    {
        return I2CStatus::InvalidAddress;
    }

    if (data == nullptr || length == 0)
    {
        return I2CStatus::InvalidArgument;
    }
    I2CStatus checkStart = SendStart(StartType::Initial);
    if (checkStart != I2CStatus::Ok)
    {
        return checkStart;
    }

    I2CStatus checkAddress = SendAddress(address, ReadOrWrite::Read);
    if (checkAddress == I2CStatus::AddressNack)
    {
        I2CStatus checkStop0 = SendStop();
        if (checkStop0 != I2CStatus::Ok)
        {
            return checkStop0;
        }
        return checkAddress;
    }
    else if (checkAddress != I2CStatus::Ok)
    {
        return checkAddress;
    }

    for (size_t index = 0; index < length; index++)
    {
        SendAck response = (index == length - 1) ? SendAck::Nack : SendAck::Ack;
        I2CStatus checkRead = ReadByte(&data[index], response);

        if (checkRead == I2CStatus::ReceiveFailed)
        {
            I2CStatus checkStop = SendStop();
            if (checkStop != I2CStatus::Ok)
            {
                return checkStop;
            }
            return checkRead;
        }
        else if (checkRead != I2CStatus::Ok)
        {
            return checkRead;
        }
    }

    I2CStatus checkStop = SendStop();
    if (checkStop != I2CStatus::Ok)
    {
        return checkStop;
    }

    return I2CStatus::Ok;
}
