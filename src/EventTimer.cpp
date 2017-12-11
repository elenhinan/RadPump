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
    Serial.println(F("Eventtimer init()"));
    #endif

    source_count = 0;
    source_ptr = NULL;
    internal_time.uint32 = 0;
    trigger_time.uint32 = 0;

    // Set stepper interrupt
    cli();//stop interrupts
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    OCR1A = 0xFFFF;// = (16*10^6) / (1*1024) - 1 (must be <65536) // todo set to board freq
    // turn on CTC mode
    //TCCR1B |= (1 << WGM12);
    // Set CS10 and CS11 bits for 64 prescaler
    TCCR1B |= (1 << CS10) | (1 << CS11);  
    // enable timer overflow interrupt
    TIMSK1 |= (1 << TOIE1);
    // enable timer compare interrupt
    //TIMSK1 |= (1 << OCIE1A);
    sei();//allow interrupts
}

ISR(TIMER1_OVF_vect)
{
    internal_time.uint16H++;
    if(trigger_time.uint16H == internal_time.uint16H && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
    {
        TIMSK1 |= (1 << OCIE1A);
    }
}

// todo: will it trigger if trigger_time.uint16h == 0?

ISR(TIMER1_COMPA_vect)
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
        TIMSK1 &= ~(1 << OCIE1A); // disable compare interrupt
    }
    OCR1A = trigger_time.uint16L;
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
        OCR1A = trigger_time.uint16L;

        if(trigger_time.uint16H == internal_time.uint16H && source_ptr != NULL) // if trigger time high-word is 0, active more fine grained interrupt
        {
            TIMSK1 |= (1 << OCIE1A);
        }
        else
        {
            TIMSK1 &= ~(1 << OCIE1A);
        }

        #ifdef DEBUG
        Serial.println(F("EventTimer primed"));
        Serial.print("  32:  ");
        Serial.println(trigger_time.uint32,HEX);
        Serial.print("  16H: ");
        Serial.println(trigger_time.uint16H,HEX);
        Serial.print("  16L: ");
        Serial.println(trigger_time.uint16L,HEX);
        Serial.print("  int: ");
        Serial.println(Now(),HEX);
        #endif
    }
}

// todo: kanskje bruke tcnt1 overflow for internal_time.uint16H++

uint32_t Now()
{
    cli();
    uint32_t value = internal_time.uint32+(uint32_t)TCNT1;
    sei();
    return value;
}

void RegisterSource(TimedEvent* source)
{
    source_list[source_count++] = source;
    #ifdef DEBUG
    Serial.print(F("EventTimer added source (0x"));
    Serial.print((uint16_t)source,HEX);
    Serial.print(F(")"));
    #endif
}

}