#include "EventTimer.h"
#include <libmaple/timer.h>

#define ET_LOWMASK uint64_t(0xFFFF)
#define ET_HIGHMASK ~ET_LOWMASK

namespace EventTimer
{
    uint32_t internal_time;
    uint32_t trigger_time_H;
    uint16_t trigger_time_L;
    uint8_t source_count;
    TimedEvent* source_ptr;
    TimedEvent* source_list[8];


void Init()
{
    internal_time = 0;
    #ifdef DEBUG
    SERIAL_DEBUG.println(F("Eventtimer init()"));
    #endif

    source_count = 0;
    source_ptr = NULL;
    internal_time = 0;
    trigger_time_H = UINT32_MAX;
    trigger_time_L = UINT16_MAX;

    SetupTimer();
}

inline void CompareEnable()
{
    timer_enable_irq(TIMER_N, TIMER_CC1_INTERRUPT);
};

inline void CompareDisable()
{
    timer_disable_irq(TIMER_N, TIMER_CC1_INTERRUPT);
};

void SetupTimer()
{
    // initialize timer used for EventTimer
    timer_init(TIMER_N);
    timer_pause(TIMER_N);
    timer_set_prescaler(TIMER_N, PRESCALE-1);

    // setup channel for overflow counting
    timer_set_reload(TIMER_N, 0xffff);
    timer_attach_interrupt(TIMER_N, TIMER_UPDATE_INTERRUPT, &TimerOverflow);
    timer_enable_irq(TIMER_N, TIMER_UPDATE_INTERRUPT);

    // setup channel for timing compare
    timer_set_compare(TIMER_N, TIMER_CH1, 0x8000);
    timer_set_mode(TIMER_N, TIMER_CH1, TIMER_OUTPUT_COMPARE);
    timer_oc_set_mode(TIMER_N, TIMER_CH1, TIMER_OC_MODE_FROZEN, 0);
    timer_attach_interrupt(TIMER_N, TIMER_CC1_INTERRUPT, &TimerCompare);
    CompareEnable();

#if TIMER_N == TIMER1
    // set timer1 interrupt priority to highest priority
    nvic_irq_set_priority(NVIC_TIMER1_UP_TIMER10, 0); // set overflow to higher priority than compare
    nvic_irq_set_priority(NVIC_TIMER1_CC, 1);
#endif

    // start timer
    timer_generate_update(TIMER_N);
    timer_resume(TIMER_N);
}

void TimerOverflow()
{
    internal_time++;
}

void TimerCompare()
{
    if(source_ptr == NULL || (trigger_time_H > internal_time))
        return;
    source_ptr->event_execute(); // trigger event
    Prime();
}

inline void Prime()
{
    uint64_t trigger_time = UINT64_MAX;
    source_ptr = NULL;
    for(int i=0;i<source_count;i++)
    {
        if(source_list[i]->event_ready)
        {

            if(source_list[i]->event_time < trigger_time)
            {
                trigger_time = source_list[i]->event_time;
                source_ptr = source_list[i];
            }
        }
    }
    trigger_time_H = (trigger_time >> 16) & UINT32_MAX;
    trigger_time_L = trigger_time & UINT16_MAX;
    timer_set_compare(TIMER_N, TIMER_CH1, trigger_time_L);

    // #ifdef DEBUG
    // uint64_t time_now = Now();
    // SERIAL_DEBUG.println(F("EventTimer primed"));
    // SERIAL_DEBUG.print("  start: ");
    // SERIAL_DEBUG.println(trigger_time,HEX);
    // SERIAL_DEBUG.print("  now(): ");
    // SERIAL_DEBUG.println(time_now,HEX);
    // #endif
}

// todo: kanskje bruke tcnt1 overflow for internal_time.uint16H++

uint64_t Now()
{
    noInterrupts();
    uint64_t value = internal_time | timer_get_count(TIMER_N);
    interrupts();
    return value;
}

void RegisterSource(TimedEvent* source)
{
    source_list[source_count++] = source;
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("EventTimer added source (0x"));
    //SERIAL_DEBUG.print((uint16_t)((void *)source),HEX);
    SERIAL_DEBUG.print(F(")"));
    #endif
}

}