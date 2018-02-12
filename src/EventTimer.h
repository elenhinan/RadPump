#pragma once
#include <Arduino.h>
#include "Config.h"

#define TIMER_N TIMER1
#define RESOLUTION 1 // resolution in microseconds
#define PRESCALE (F_CPU*RESOLUTION/1000000L)
#define DELTA_T (float(PRESCALE)/float(F_CPU))
#define MICROS ((1000000*PRESCALE)/F_CPU)
#define DELAY 2000 // us delay before movement starts "now", default 1ms

class TimedEvent
{
public:
    uint64_t event_time = UINT64_MAX;
    uint32_t event_iteration = 0;
    bool event_ready = false;
    virtual void event_execute() = 0;
    TimedEvent();
};

// move engine
namespace EventTimer
{
    // variables
    static const float dt = DELTA_T;
    static const uint64_t delay = DELAY/MICROS;

    // functions
    extern void init();
    extern void prime();
    extern uint64_t now();
    extern void register_source(TimedEvent* source);
};