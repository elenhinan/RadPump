#pragma once
#include <arduino.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include "LinearStage.h"

// Syringe to/from eeprom
#define SYRINGE_MAX 8
#define SYRINGE_ADDR_START 0x20
#define SYRINGE_ADDR_END (SYRINGE_ADDR_START + SYRINGE_MAX * sizeof(Syringe)

class Syringe
{
private:
    int8_t index;      // index in eeprom
    char name[16];      // short name
    float ml_per_mm;    // volume/length
    float empty;        // mm from home when empty
    float full;         // mm from home when full
    float max_speed;    // max safe speed before stall
public:
    Syringe();
    Syringe(int8_t index);
    Syringe(String name, float ml_per_mm, float empty, float full, float max_speed);
    void load(int8_t index);
    void save(int8_t index);
    char* get_name() { return &name; }
    //void set_name(char* value) { value.toCharArray(name, 16); }
};

class Injector
{
    // variables
public:
    static const uint8_t VOLUME = 0;
    static const uint8_t ACTIVITY = 1;
private:
    Syringe syringe;
    LinearStage* linearstage;
    Adafruit_GFX* display;
    float planned_amount;
    float injected_amount;
    float planned_time;

    // functions
private:

    
public:
    Injector(LinearStage* linearstage, Adafruit_GFX* display);
    void update_display();

};