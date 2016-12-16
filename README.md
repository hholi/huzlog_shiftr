# ESP8266 logger and monitor #

Simple environment sense, message pub/sub and monitoring.

Using ESP8266, HDC1000, OLED.
Shiftr.io as MQTT message broker.


### User stories

* Running on battery
* Deep sleep
* Small case for sensor module
* TDD


### Running tests
Based on Google test
Install and build with (https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/)

'sudo apt-get install libgtest-dev

sudo apt-get install cmake # install cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
 
# copy or symlink libgtest.a and libgtest_main.a to your /usr/lib folder
sudo cp *.a /usr/lib'

The run tests with:
'test.sh'


### Reference docs

http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://learn.adafruit.com/adafruit-hdc1008-temperature-and-humidity-sensor-breakout/wiring-and-test?view=all
