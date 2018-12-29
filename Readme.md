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

## Dependencies

The library itself doesn't require any dependencies apart of the standard C library. However, to compile example or tests cmake and cmocka are required. Examples and tests compilation has been tested on the Ubuntu 18.4 and macOS Mojave.

Ubuntu:
```bash
sudo apt install cmake
sudo apt install libcmocka-dev
```

OSX (using brew):
```bash
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
brew install cmake
brew install cmocka
```

## Build Examples

```bash
mkdir -p ./examples/build
cd ./examples/build
cmake ..
make
./example
```

## Build Tests

```bash
mkdir -p ./tests/build
cd ./tests/build
cmake ..
make
./tests
```

## TODO

- requests retry
- passs user data in all callbacks
- request callbacks, in case of error there is no info about message which failed (do we need this?)
- In case of set command, validate reply and check if sensor returned expected value.
