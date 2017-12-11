#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <EEPROM.h>
#include "LinearStage.h"

// Syringe to/from eeprom
#define SYRINGE_MAX 8
#define SYRINGE_ADDR_START 0x20
#define SYRINGE_ADDR_END (SYRINGE_ADDR_START + SYRINGE_MAX * sizeof(Syringe)

class Syringe
{
// variables
public:
    int8_t index;       // index in eeprom
    char name[16];      // short name
    float cal_factor;   // calibration factor for (full-empty)/volume
    float volume;       // syringe volume
    float empty;        // mm from home when empty
    float full;         // mm from home when full
    float mm_per_ml;
    float ml_per_mm;
// functions
private:
    int8_t get_addr() { return SYRINGE_ADDR_START + sizeof(Syringe)*index; }
public:
    Syringe();
    void save();
    void load();
    void recalc();

};

class Injector
{
// consts
public:
    static const uint8_t MODE_VOLUME = 0;
    static const uint8_t MODE_ACTIVITY = 1;
    static const uint8_t STATE_INSERT = 0;
    static const uint8_t STATE_AUTO = 1;
    static const uint8_t STATE_READY = 2;
    static const uint8_t STATE_CONFIG = 3;
    static const uint8_t STATE_RUNNING = 4;
    static const uint8_t STATE_EMPTY = 5;


// variables
private:
    Syringe syringe;
    LinearStage* linearstage;
    Adafruit_GFX* display;
    uint8_t state;
    float planned_amount;
    float planned_time;
    float start_amount;
    float start_time;
    //float activity_hl;      // halflife
    //uint32_t activity_t0;   // activity start time
    //float activity_a0;      // activity in bq at start time
    //uint32_t activity_t;    // time for last activity calculation
    //float activity_a;       // activity
    uint8_t mode;

// functions
private:
    void print_volume(float volume);
    void print_activity(float volume);
    void draw_syringe(float volume);
    //float activity_a();
    
public:
    Injector(LinearStage* linearstage, Adafruit_GFX* display);
    void move_insertion();
    void move_max();
    void auto_insert();
    void update_display();
    void set_activity(float a0, uint32_t t0);
};