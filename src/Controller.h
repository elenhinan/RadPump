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
    "injector": "1",
    "isotope": {
        "name": "F18",
        "halflife": "109.7",
        "t0": "20180211-151035.23"
    },
    "syringe": {
        "name":"Omnifix 1ml",
        "volume":"1.0f",
        "end":"14.5",
        "start":"71.8f"
    },
    "injection": {
        "time": "20180211-152035.32",
        "amount": "20",
        "unit": "ml",
        "speed":"0.01",
        "duration":"150"
    },
    "command":{
        "cmd":"move",
        "param":"20"
    }
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