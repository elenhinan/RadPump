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
};

// move engine
namespace EventTimer
{
    // variables
    extern uint32_t internal_time;
    extern uint8_t source_count;
    static const float dt = DELTA_T;
    static const uint64_t delay = DELAY/MICROS;
    extern uint32_t trigger_time_H;
    extern uint16_t trigger_time_L;
    extern TimedEvent* source_ptr;
    extern TimedEvent* source_list[8];

    // functions
    extern void Init();
    extern void SetupTimer();
    extern void Prime();
    extern uint64_t Now();
    extern void RegisterSource(TimedEvent* source);
    extern void TimerOverflow();
    extern void TimerCompare();
};