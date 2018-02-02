#include <Arduino.h>
#include <Adafruit_GFX_AS.h>    // Core graphics library
#include <Adafruit_ST7735.h>
//#include <TimeLib.h>
//#include <DS1307RTC.h>
#include "Config.h"
#include "pins.h"
#include "LinearStage.h"
#include "EventTimer.h"
#include "Injector.h"

#define BAUDRATE 115200

LinearStage linearstageA(STEPA_EN, STEPA_STEP, STEPA_CS, STEPA_STALL, 'A');
//LinearStage linearstageB(STEPB_EN, STEPB_STEP, STEPB_CS, STEPB_STALL, 'B');

Adafruit_ST7735 display(DISP_CS, DISP_DC, DISP_RST);

Injector injectorA(&linearstageA, &display);

enum LoopState {
    STATE_INIT,
    STATE_IDLE,
    STATE_HOMING,
    STATE_JOG_NEG,
    STATE_JOG_POS,
    STATE_SETUP
};

LoopState loop_state;

void setup() {

    loop_state = STATE_INIT;
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_POS, INPUT_PULLUP);
    pinMode(BUTTON_NEG, INPUT_PULLUP);

    // turn on backlight
    pinMode(DISP_BL, OUTPUT);
    //digitalWrite(DISP_BL, HIGH);
    analogWrite(DISP_BL, 0x64);

    // setup Serial
    SERIAL_DEBUG.begin(BAUDRATE);
    //#ifdef ARDUINO_SAMD_ZERO
    //while(!Serial);
    //#endif
    SERIAL_DEBUG.println("Start");

    // setup RTC
    //setSyncProvider(RTC.get);   // the function to get the time from the RTC

    // get the date and time the compiler was run
    //if(timeStatus()!= timeSet) 
    //    SERIAL_DEBUG.println(F("RTC: failed"));
    //else
    //    SERIAL_DEBUG.println(F("RTC: success"));      
    // if (rtc.lostPower()) {
    //     SERIAL_DEBUG.println(F("RTC lost power, lets set the time!"));
    //     // following line sets the RTC to the date & time this sketch was compiled
    //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     // This line sets the RTC with an explicit date & time, for example to set
    // }

    
    // setup display
    display.initR(INITR_BLACKTAB);
    display.fillScreen(ST7735_BLACK);
    delay(1000);

    // joystick setup
    //analogReference(DEFAULT);
    //pinMode(JSTK_X, INPUT);
    //pinMode(JSTK_Y, INPUT);
    //pinMode(JSTK_SW, INPUT_PULLUP);

    // init steppers
    linearstageA.init();
    //linearstageB.init();

    EventTimer::Init();
    EventTimer::RegisterSource(&linearstageA);

    // enable both drivers
    linearstageA.enable();
    //linearstageB.enable();

    injectorA.init();
    injectorA.set_isotope(3);
    injectorA.set_activity(10.0f, now(), 0.6);

    linearstageA.home(LinearStage::DIR_NEG);
    linearstageA.move_abs(50, 6., 4.0);
    linearstageA.wait_move();
    //linearstageA.move_abs(20, 2., 4.0);
}

void loop() {
    //display.fillScreen(ST7735_BLACK);
    //injectorA.update_display();
    delay(10);
    static bool button_pos_last = true;
    static bool button_neg_last = true;
    bool button_neg_state = digitalRead(BUTTON_NEG);
    bool button_pos_state = digitalRead(BUTTON_POS);
    if(button_neg_state != button_neg_last)
    {
        #ifdef DEBUG
        SERIAL_DEBUG.print(F("Button(-):"));
        SERIAL_DEBUG.println(button_neg_state);
        SERIAL_DEBUG.println((uint32)EventTimer::source_ptr);
        SERIAL_DEBUG.println(EventTimer::trigger_time_H);
        SERIAL_DEBUG.println(EventTimer::Now());
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
        SERIAL_DEBUG.println((uint32)EventTimer::source_ptr);
        SERIAL_DEBUG.println(EventTimer::trigger_time_H);
        SERIAL_DEBUG.println(EventTimer::Now());
        #endif
        if(button_pos_state)
            linearstageA.stop();
        else
            linearstageA.move_abs(70., MANUAL_SPEED, MANUAL_ACCEL);
    }
    button_neg_last = button_neg_state;
    button_pos_last = button_pos_state;
    
}