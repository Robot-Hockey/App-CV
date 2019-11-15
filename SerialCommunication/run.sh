#! /bin/bash
sudo chmod a+wr /dev/ttyUSB0
g++ -o test test.cpp mySerial.cpp
