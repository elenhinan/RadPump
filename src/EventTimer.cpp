#include "EventTimer.h"
#include <libmaple/timer.h>

#define ET_LOWMASK uint64_t(0xFFFF)
#define ET_HIGHMASK ~ET_LOWMASK
#define ET_MAX 0xFFFFFFFFFFFFFFFF

namespace EventTimer
{
    uint64_t internal_time;
    uint64_t trigger_time;
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
    trigger_time = 0;

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
    timer_set_compare(TIMER_N, TIMER_CH1, 0x0000);
    timer_set_mode(TIMER_N, TIMER_CH1, TIMER_OUTPUT_COMPARE);
    timer_oc_set_mode(TIMER_N, TIMER_CH1, TIMER_OC_MODE_FROZEN, 0);
    timer_attach_interrupt(TIMER_N, TIMER_CC1_INTERRUPT, &TimerCompare);
    CompareDisable();

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
    internal_time += 1<<16;
    if((trigger_time & ET_HIGHMASK) == (internal_time & ET_HIGHMASK) && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
    {
        CompareEnable();
    }
}

// todo: will it trigger if trigger_time.uint16h == 0?

void TimerCompare()
{
    source_ptr->event_execute(); // trigger event

    // if events are skipped because trigger_time.uint16h == 0 && trigger_time.uint16l == tcnt1
    // add do{} while(trigger_time = Now())
    // should not be needed as "OCR1A/B is compared with TCNT1 value at all time"

    // find time of next event
    trigger_time = ET_MAX;
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
    if ((trigger_time & ET_HIGHMASK) > (internal_time & ET_HIGHMASK) || source_ptr == NULL) // if trigger due after more than 65535 ticks or source_ptr == NULL
    {
        CompareDisable();
    }
    timer_set_compare(TIMER_N, TIMER_CH1, trigger_time & ET_LOWMASK);
}

void Prime()
{
    if(source_ptr == NULL) // only find next event if none in queue
    {
        trigger_time = ET_MAX;

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
        timer_set_compare(TIMER_N, TIMER_CH1, trigger_time & ET_LOWMASK);

        if((trigger_time & ET_HIGHMASK) == (internal_time & ET_HIGHMASK) && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
        {
            CompareEnable();
        }
        else
        {
            CompareDisable();
        }

        #ifdef DEBUG
        SERIAL_DEBUG.println(F("EventTimer primed"));
        SERIAL_DEBUG.print("  start:  ");
        SERIAL_DEBUG.println(trigger_time,HEX);
        SERIAL_DEBUG.print("  now(): ");
        SERIAL_DEBUG.println(Now(),HEX);
        #endif
    }
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