# Bare-Metal AVR I2C Driver

A small, blocking I2C driver written in C++ for the ATmega32U4 (Arduino Leonardo). Instead of using the Arduino `Wire` library, it communicates with the AVR TWI hardware directly through its registers.

I built it as a learning project to understand what `Wire` does behind the scenes. It is my first complete register-level communication driver, so the goal is learning rather than replacing a production library.

## Current Status

The driver is complete, compiles, and has been validated on real hardware: it is used in my [Gesture Glove](https://github.com/Yamin-Osman/Gesture-Glove) project to communicate with an MPU6050 IMU in place of `Wire.h`.

It currently supports:

- Initializing the AVR TWI peripheral
- Writing bytes to an I2C device
- Reading bytes from an I2C device
- Combined write-read transactions
- Repeated START conditions
- ACK and NACK handling
- Timeouts
- Basic argument and address validation
- Error reporting through an `I2CStatus` enum

An earlier set of IMU modules was unreliable, so I did not use them as my test hardware. The driver's hardware validation comes from the Gesture Glove build.

## Hardware and Tools

- Arduino Leonardo
- ATmega32U4 microcontroller
- MPU6050 IMU (used in the Gesture Glove project)
- PlatformIO
- Visual Studio Code
- C++

## Why I Built It

Arduino libraries make it easy to communicate with hardware, but they can also hide what the microcontroller is actually doing.

For this project, I wanted to learn:

- How the AVR TWI peripheral works
- How START and STOP conditions are generated
- How an address is combined with the read/write bit
- How ACK and NACK responses are handled
- How repeated START works
- How hardware status codes are checked
- How timeouts prevent the program from waiting forever
- How to design a small reusable driver API

## Public Functions

The driver provides four main functions:

```cpp
I2CInit();
I2CWrite();
I2CRead();
I2CWriteRead();
```

### `I2CInit()`

Configures the AVR TWI hardware and sets the I2C clock frequency.

### `I2CWrite()`

Sends one or more bytes to an I2C device.

### `I2CRead()`

Reads one or more bytes from an I2C device.

### `I2CWriteRead()`

Writes data to a device, generates a repeated START, and then reads data without releasing the bus.

## Basic Transaction Flow

### Write transaction

```text
START
→ Device address + Write
→ Data
→ STOP
```

### Read transaction

```text
START
→ Device address + Read
→ Receive data
→ NACK final byte
→ STOP
```

### Combined write-read transaction

```text
START
→ Device address + Write
→ Register address or command
→ Repeated START
→ Device address + Read
→ Receive data
→ STOP
```

## Internal Design

The driver uses private helper functions for operations such as:

- Waiting for the TWI interrupt flag
- Sending START
- Sending STOP
- Sending a device address
- Writing one byte
- Reading one byte

These helpers are placed in an anonymous namespace so they are only available inside the driver source file.

TWI status values and masks are stored as named `constexpr` constants.

## Timeout Handling

The AVR hardware normally sets the `TWINT` flag when an operation finishes.

If there is a wiring problem or the hardware does not respond correctly, waiting for this flag forever would freeze the program.

To avoid that, the driver uses a maximum loop count and returns a timeout status if the operation does not complete.

## Usage Example

See the [Gesture Glove](https://github.com/Yamin-Osman/Gesture-Glove) repository for a real use of this driver. It is included there under `lib/BareI2C/` and used to read IMU data instead of `Wire.h`.

## What I Learned

While building this project, I practiced:

- Reading an AVR datasheet
- Working with memory-mapped registers
- Using bitwise operations
- Reading hardware status codes
- Designing public and private APIs
- Using `enum class`
- Using `constexpr`
- Using an anonymous namespace
- Checking pointers and arguments
- Propagating errors between functions
- Using the driver as a real dependency in another project
- Using Git commits to track project milestones

## Project Structure

```text
BareMetalI2C/
├── lib/
│   └── BareI2C/
│       ├── BareI2C.h
│       └── BareI2C.cpp
├── src/
│   └── main.cpp
├── platformio.ini
├── .gitignore
└── README.md
```

## Limitations

- The driver is blocking rather than interrupt-driven.
- It has been validated on one I2C device (an MPU6050 IMU), not across a range of devices.
- It only supports 7-bit I2C addresses.
- It does not automatically retry failed transactions.
- It does not currently perform bus recovery.
- It has only been written for the ATmega32U4.
- It has not been tested in a multi-controller system.

## Future Work

- Test against additional I2C devices (for example, an SSD1306 OLED display)
- Add a simple example program that scans for devices and reads a register
- Capture transactions with a logic analyzer to verify timing and signal behavior
- Add automated tests where possible
- Improve bus recovery and error handling

## Author

Built by Yamin Osman, a McMaster University Engineering student, as part of learning embedded systems, C++, and register-level microcontroller programming.
