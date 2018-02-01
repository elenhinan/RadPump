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

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);

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

    //linearstageA.calibrate();

    //delay(2000);
    injectorA.init();
    injectorA.set_isotope(3);
    injectorA.set_activity(10.0f, now(), 0.6);

    linearstageA.home(LinearStage::DIR_NEG);
    linearstageA.move_abs(50, 6., 4.0, 1);
    linearstageA.wait_move();
    //linearstageA.search();
    linearstageA.move_abs(20, 2., 4.0, 1);
    //delay(5000);
    //linearstageA.search();
}

void loop() {
    //display.fillScreen(ST7735_BLACK);
    injectorA.update_display();
    delay(10);
    //static int count = 0;
    //digitalWrite(LED_BUILTIN, HIGH);
    //delay(50);
    //digitalWrite(LED_BUILTIN, LOW);
    //digitalWrite(STEPA_STEP, HIGH);
    //delay(1);
    //digitalWrite(STEPA_STEP, LOW);
    //delay(50);
    //display.fillScreen(ST7735_BLACK);
    //injectorA.update_display();
    //delay(500);
    //display.setCursor(0, 0);
    // display.setTextColor(ST7735_GREEN, ST7735_BLACK);
    // display.print(linearstageA.get_position_mm(), DEC);
    // display.println("mm");
    // display.print(EventTimer::Now(), HEX);
    //display.println("ticks");
}