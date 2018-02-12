#pragma once
#include <Arduino.h>
#include <Adafruit_GFX_AS.h>
#include <TimeLib.h>
#include "LinearStage.h"
#include "Config.h"

typedef struct
{
    char name[8]; // up to 7 char
    float halflife; // in minutes
} Isotope;

typedef struct
{
    char name[16];      // short name
    float volume;       // syringe volume
    float empty;        // mm from home when empty
    float full;         // mm from home when full
} Syringe;

const Isotope IsotopesEeprom[ISOTOPE_MAX] PROGMEM = {
    {"F-18", 109.7f},
    {"C-11", 20.4f},
    {"Ga-68", 67.71f},
    {"N-13", 9.97f},
    {"Zr-89", 78.41f*60},
    {"In-111", 2.8049f*24*60}
};

const Syringe SyringesEeprom[SYRINGE_MAX] PROGMEM = {
    {"Omnifix 1ml", 1.0f, 14.5f, 71.8f}
};

enum InjectorMode : uint8_t
{
    MODE_VOLUME,
    MODE_ACTIVITY,
    MODE_DISTANCE
};

enum InjectorState : uint8_t
{
    STATE_INSERT,
    STATE_AUTO,
    STATE_READY,
    STATE_RUNNING,
    STATE_TIMER,
    STATE_EMPTY,
    STATE_CONFIG_SYRINGE,
    STATE_CONFIG_VOLUME,
    STATE_CONFIG_ACTIVITY,
    STATE_CONFIG_INJECT,
    STATE_CONFIG_RATE
};

class Injector
{
// variables
private:
    Syringe syringe;
    LinearStage* linearstage;
    InjectorState state;
    float planned_amount;
    float planned_rate;
    float prev_vol;
    //time_t planned_start;
    Isotope isotope;
    //uint8_t activity_isotope;
    time_t activity_t0;     // activity start time
    float activity_c0;      // concentration in MBq at start time
    //uint32_t activity_t;    // time for last activity calculation
    //float activity_a;       // activity
    InjectorMode mode;

// functions
private:
    //void print_name();
    //void print_volume(float volume);
    //void print_activity(float volume);
    //void print_plan();
    //void draw_syringe(float volume, bool redraw);
    
public:
    Injector(LinearStage* linearstage);
    void init();
    void move_insertion();
    void move_max();
    void auto_insert();
    //void update_display();
    float get_volume();
    float get_activity();
    void set_isotope(uint8_t index);
    void set_syringe(uint8_t index);
    //void set_isotope(char name[8], float halflife);
    void set_activity(float a0, time_t t0, float volume);
    void inject_volume(float vol, float rate);
    void inject_activity(float vol, float rate);
    void inject_distance(float dist, float rate);
    void start();
    void delay_start(uint32_t time);
};