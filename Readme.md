## SDS011 Dust Sensor Library

Low level SDS011 dust sensor library written in C. It should be possible to compile this library on any target by implementing the following functions:

- millis

  milliseconds since program start, this function is used to handle communication timeouts.
- bytes_available

  number of bytes available in the serial input queue.
- read_byte

  read one byte from the serial input queue.
- send_byte

  send one byte to the dust sensor.

More information can be found in the `examples/example.c` file.

## Main features

- Can support multipe sensors connected to the same port (with RS485), or multiple sensors connected to multiple ports.
- Fully asynchronous, callback based api.
- Retransmissions in case of failure or timeout.
- Messages queue.
- Easily portable to new targets.

## Dependencies

The library itself doesn't require any dependencies apart of the standard C library. However, to compile example or tests a few additional dependencies are required. Compilation has been tested on the Ubuntu 18.4 and macOS Mojave.

Ubuntu:
```bash
sudo apt install cmake
sudo apt install libcmocka-dev
sudo apt install lcov
sudo apt install doxygen
sudo apt install ninja
```

OSX:
```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
brew install cmake
brew install cmocka
brew install lcov
brew install doxygen
brew install ninja
```

## Build

```bash
mkdir ./build
cd ./build
cmake -GNinja ..
cmake --build .
ninja test
ninja coverage
open ./tests/coverage/index.html
./examples/example
```
