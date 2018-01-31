#pragma once
#include <Arduino.h>
#include "Config.h"

#define TIMER_N TIMER1
#define PRESCALE 72
#define DELTA_T (float(PRESCALE)/float(F_CPU))

union uint32_split_t
{
    uint32_t uint32;
    struct {
        uint16_t uint16L;
        uint16_t uint16H;
    };
};

class TimedEvent
{
public:
    uint32_t event_time;
    bool event_ready;
    virtual void event_execute() = 0;
};

// move engine
namespace EventTimer
{
    // variables
    extern uint32_split_t internal_time;
    extern uint8_t source_count;
    static const float dt = DELTA_T;
    extern uint32_split_t trigger_time;
    extern TimedEvent* source_ptr;
    extern TimedEvent* source_list[8];

    // functions
    extern void Init();
    extern void SetupTimer();
    extern void Prime();
    extern uint32_t Now();
    extern void RegisterSource(TimedEvent* source);
    extern void TimerOverflow();
    extern void TimerCompare();
};