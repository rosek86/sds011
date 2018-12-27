TODO:
  - more examples
  - readme

Dependencies
Ubuntu:
apt install libcmocka-dev
OSX:
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
brew install cmocka

Build Tests

mkdir ./tests/build
cd ./tests/build
cmake ..
make
./tests
