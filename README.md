# Bare-Metal AVR I2C Driver

This is a small I2C driver that I wrote in C++ for the ATmega32U4 on an Arduino Leonardo.

I built it as a learning project because I wanted to understand what the Arduino `Wire` library does behind the scenes. Instead of using `Wire.h`, this driver communicates with the AVR TWI hardware directly through its registers.

This is my first complete register-level communication driver, so the main goal of the project is learning rather than replacing an existing production library.

## Current Status

The first version of the driver is complete and compiles successfully.

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

Hardware testing is still in progress.

My original MPU6050 modules were unreliable, so I decided not to use them as the main test hardware. I have ordered SSD1306 OLED displays and will use one to test the write side of the driver.

## Hardware and Tools

- Arduino Leonardo
- ATmega32U4 microcontroller
- SSD1306 128×64 I2C OLED display
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

The driver currently provides four main functions:

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

Writes data to a device, generates a repeated START and then reads data without releasing the bus.

## Basic Transaction Flow

### Write transaction


START
→ Device address + Write
→ Data
→ STOP


### Read transaction


START
→ Device address + Read
→ Receive data
→ NACK final byte
→ STOP


### Combined write-read transaction


START
→ Device address + Write
→ Register address or command
→ Repeated START
→ Device address + Read
→ Receive data
→ STOP


## Internal Design

The driver uses private helper functions for operations such as:

- Waiting for the TWI interrupt flag
- Sending START
- Sending STOP
- Sending a device address
- Writing one byte
- Reading one byte

These helpers are placed in an anonymous namespace so that they are only available inside the driver source file.

TWI status values and masks are stored as named `constexpr` constants.

## Timeout Handling

The AVR hardware normally sets the `TWINT` flag when an operation finishes.

If there is a wiring problem or the hardware does not respond correctly, waiting for this flag forever would freeze the program.

To avoid that, the driver uses a maximum loop count and returns a timeout status if the operation does not complete.

## Planned OLED Test

When the SSD1306 displays arrive, I plan to test the driver in stages:

1. Check whether the display acknowledges its I2C address.
2. Send the SSD1306 initialization commands.
3. Clear the display.
4. Draw a simple pattern or text.
5. Use a logic analyzer later to inspect the I2C signals.


The OLED will verify the initialization and write portions of the driver. A different I2C device will be needed later to fully verify reading and combined write-read transactions.

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
- Using Git commits to track project milestones

## Project Structure


BareMetalI2C/
├── lib/
│ └── BareI2C/
│ ├── BareI2C.h
│ └── BareI2C.cpp
├── src/
│ └── main.cpp
├── platformio.ini
├── .gitignore
└── README.md


## Limitations

This is currently a learning project and has several limitations:

- The driver is blocking rather than interrupt-driven.
- Hardware testing is not yet complete.
- It only supports 7-bit I2C addresses.
- It does not automatically retry failed transactions.
- It does not currently perform bus recovery.
- It has only been written for the ATmega32U4.
- It has not been tested in a multi-controller system.

## Future Work

- Test `I2CWrite()` using the SSD1306 OLED
- Test `I2CRead()` with a reliable readable device
- Test repeated START using `I2CWriteRead()`
- Add a simple SSD1306 example
- Capture transactions using a logic analyzer
- Add automated tests where possible
- Improve bus recovery and error handling

## Author

Built by an incoming first-year engineering student as part of learning embedded systems, C++ and register-level microcontroller programming.