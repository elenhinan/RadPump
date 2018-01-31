#pragma once

#include <Arduino.h>

class SerialControl
{
private:
    uint8_t state;
public:
    SerialControl(HardwareSerial port);
    void SerialName(char name);
    void ParseCommand(HardwareSerial &port);
    
};