#include "EventTimer.h"

namespace EventTimer
{
    uint32_split_t internal_time;
    uint32_split_t trigger_time;
    uint8_t source_count;
    TimedEvent* source_ptr;
    TimedEvent* source_list[8];


void Init()
{
    internal_time.uint32 = 0;
    #ifdef DEBUG
    SERIAL_DEBUG.println(F("Eventtimer init()"));
    #endif

    source_count = 0;
    source_ptr = NULL;
    internal_time.uint32 = 0;
    trigger_time.uint32 = 0;

    SetupTimer();
}

inline void CompareEnable()
{
    timer_enable_irq(TIMER_N, TIMER_CC2_INTERRUPT);
};

inline void CompareDisable()
{
    timer_disable_irq(TIMER_N, TIMER_CC2_INTERRUPT);
};

void SetupTimer()
{
    // initialize timer used for EventTimer
    timer_init(TIMER_N);
    timer_pause(TIMER_N);
    timer_set_prescaler(TIMER_N, PRESCALE-1);

    // setup channel for overflow counting
    timer_set_compare(TIMER_N, TIMER_CH1, 0x0000);
    timer_set_mode(TIMER_N, TIMER_CH1, TIMER_OUTPUT_COMPARE);
    timer_oc_set_mode(TIMER_N, TIMER_CH1, TIMER_OC_MODE_FROZEN, 0);
    timer_attach_interrupt(TIMER_N, TIMER_CC1_INTERRUPT, &TimerOverflow);
    timer_enable_irq(TIMER_N, TIMER_CC1_INTERRUPT);

    // setup channel for timing compare
    timer_set_compare(TIMER_N, TIMER_CH2, 0x0000);
    timer_set_mode(TIMER_N, TIMER_CH2, TIMER_OUTPUT_COMPARE);
    timer_oc_set_mode(TIMER_N, TIMER_CH2, TIMER_OC_MODE_FROZEN, 0);
    timer_attach_interrupt(TIMER_N, TIMER_CC2_INTERRUPT, &TimerCompare);
    timer_enable_irq(TIMER_N, TIMER_CC2_INTERRUPT);

    CompareDisable();

    // start timer
    timer_generate_update(TIMER_N);
    timer_resume(TIMER_N);
}

void TimerOverflow()
{
    internal_time.uint16H++;
    if(trigger_time.uint16H == internal_time.uint16H && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
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
    trigger_time.uint32 = 0xffffffff;
    source_ptr = NULL;

    for(int i=0;i<source_count;i++)
    {
        if(source_list[i]->event_ready)
        {
            if(source_list[i]->event_time < trigger_time.uint32)
            {
                trigger_time.uint32 = source_list[i]->event_time;
                source_ptr = source_list[i];
            }
        }
    }
    if (trigger_time.uint16H > internal_time.uint16H || source_ptr == NULL) // if trigger due after more than 65535 ticks or source_ptr == NULL
    {
        CompareDisable();
    }
    timer_set_compare(TIMER_N, TIMER_CH2, trigger_time.uint16L);
}

void Prime()
{
    if(source_ptr == NULL) // only find next event if none in queue
    {
        trigger_time.uint32 = 0xffffffff;

        for(int i=0;i<source_count;i++)
        {
            if(source_list[i]->event_ready)
            {

                if(source_list[i]->event_time < trigger_time.uint32)
                {
                    trigger_time.uint32 = source_list[i]->event_time;
                    source_ptr = source_list[i];
                }
            }
        }
        timer_set_compare(TIMER_N, TIMER_CH2, trigger_time.uint16L);

        if(trigger_time.uint16H == internal_time.uint16H && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
        {
            CompareEnable();
        }
        else
        {
            CompareDisable();
        }

        #ifdef DEBUG
        SERIAL_DEBUG.println(F("EventTimer primed"));
        SERIAL_DEBUG.print("  32:  ");
        SERIAL_DEBUG.println(trigger_time.uint32,HEX);
        SERIAL_DEBUG.print("  16H: ");
        SERIAL_DEBUG.println(trigger_time.uint16H,HEX);
        SERIAL_DEBUG.print("  16L: ");
        SERIAL_DEBUG.println(trigger_time.uint16L,HEX);
        SERIAL_DEBUG.print("  int: ");
        SERIAL_DEBUG.println(Now(),HEX);
        #endif
    }
}

// todo: kanskje bruke tcnt1 overflow for internal_time.uint16H++

uint32_t Now()
{
    noInterrupts();
    uint32_t value = internal_time.uint32 + (uint32_t)timer_get_count(TIMER_N);
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