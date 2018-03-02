#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include "Config.h"

/* Example
use: http://arduinojson.org/assistant/

commands: move, stop, jog, home, return
type: volume, length, activity
unit: 0:"ml" / 1:"MBq"/ 1:"mm" unit

{
    "injector": [
        {
            "isotope": {
                "name": "F18",                      string
                "halflife": "109.7",                s
                "t0": "20180211-151035.23"          datetime
            },
            "syringe": {
                "name":"Omnifix 1ml",               string
                "volume":"1000.0f",                 ul
                "end":"14.5",                       mm
                "start":"71.8",                     mm
                "current":"53.3"                    mm
            },
            "injection": {
                "delay": "30",                      s
                "amount": "20.0",                   ul / ul / mm
                "mode": "vol",                      vol / act / len
                "duration":"150"                    s
            }
        }
    ]
}
*/

class Controller
{
private:
    StaticJsonBuffer<JSONBUFFER_SIZE> jsonBuffer;
    uint8_t state;
    HardwareSerial* stream;
public:
    Controller(HardwareSerial* stream);
    void rxPacket();
    void txPacket();
};