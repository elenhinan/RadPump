#include "main.h"

void setup() {

    loop_state = STATE_INIT;
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_POS, INPUT_PULLUP);
    pinMode(BUTTON_NEG, INPUT_PULLUP);

    // setup Serial
    #ifdef DEBUG
    SERIAL_DEBUG.begin(BAUDRATE);
    SERIAL_DEBUG.println("Start");
    #endif

    // setup RTC
    RTC.begin(RTC_CS);
    RTC.set(571246347);
    setSyncProvider(RTC.get);

    // setup event system
    EventTimer::init();

    // init steppers
    linearstageA.init();
    //linearstageB.init();

    // enable both drivers
    linearstageA.enable();
    //linearstageB.enable();

    // init injector engine
    injectorA.init();
    //injectorB.init();
    
    
    // demo stuff:
    injectorA.set_isotope(3);
    injectorA.set_activity(10.0f, now(), 0.6);

    linearstageA.home(LinearStage::DIR_NEG);
    linearstageA.move_abs(50, 12., 4.0);
    linearstageA.wait_move();
    //linearstageA.move_abs(20, 2., 4.0);
}

void check_buttons()
{
    static bool button_pos_last = true;
    static bool button_neg_last = true;
    bool button_neg_state = digitalRead(BUTTON_NEG);
    bool button_pos_state = digitalRead(BUTTON_POS);
    if(button_neg_state != button_neg_last)
    {
        #ifdef DEBUG
        SERIAL_DEBUG.print(F("Button(-):"));
        SERIAL_DEBUG.println(button_neg_state);
        #endif
        if(button_neg_state)
            linearstageA.stop();
        else
            linearstageA.move_abs(0., MANUAL_SPEED, MANUAL_ACCEL);
    }
    if(button_pos_state != button_pos_last)
    {
        #ifdef DEBUG
        SERIAL_DEBUG.print(F("Button(+):"));
        SERIAL_DEBUG.println(button_pos_state);
        #endif
        if(button_pos_state)
            linearstageA.stop();
        else
            linearstageA.move_abs(70., MANUAL_SPEED, MANUAL_ACCEL);
    }
    button_neg_last = button_neg_state;
    button_pos_last = button_pos_state;
}

void loop() {
    //delay(1000);
    check_buttons();
    //tmElements_t tm;
    //RTC.read(tm);
    //SERIAL_DEBUG.print("RTC:");
    //SERIAL_DEBUG.print(tm.Hour);
    //SERIAL_DEBUG.print(":");
    //SERIAL_DEBUG.print(tm.Minute);
    //SERIAL_DEBUG.print(":");
    //SERIAL_DEBUG.print(tm.Second);
    //SERIAL_DEBUG.print("  ");
    //SERIAL_DEBUG.print(tm.Year);
    //SERIAL_DEBUG.print("-");
    //SERIAL_DEBUG.print(tm.Month);
    //SERIAL_DEBUG.print("-");
    //SERIAL_DEBUG.print(tm.Day);
    //SERIAL_DEBUG.println("");
}