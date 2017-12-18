#include <Arduino.h>
#include <Adafruit_SH1106.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "pins.h"
#include "LinearStage.h"
#include "EventTimer.h"
#include "Injector.h"

#define BAUDRATE 115200

LinearStage linearstageA(STEP_EN, STEPA_STEP, STEPA_CS, STEPA_STALL, 'A');
//LinearStage linearstageB(STEP_EN, STEPB_STEP, STEPB_CS, STEPB_STALL, 'B');

Adafruit_SH1106 displayA(OLED_DC, OLED_RST, OLEDA_CS);
//Adafruit_SH1106 displayB(OLED_DC, OLED_RST, OLEDB_CS);

Injector injectorA(&linearstageA, &displayA);
//Injector injectorB(linearstageB, displayB);

void setup() {
    // disable both drivers before setup
    pinMode(STEP_EN, OUTPUT);
    digitalWrite(STEP_EN, HIGH); // disable drivers

    // setup serial
    Serial.begin(BAUDRATE);

    // setup RTC
    setSyncProvider(RTC.get);   // the function to get the time from the RTC

    // get the date and time the compiler was run
    if(timeStatus()!= timeSet) 
        Serial.println(F("RTC: failed"));
    else
        Serial.println(F("RTC: success"));      
    // if (rtc.lostPower()) {
    //     Serial.println(F("RTC lost power, lets set the time!"));
    //     // following line sets the RTC to the date & time this sketch was compiled
    //     rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //     // This line sets the RTC with an explicit date & time, for example to set
    // }

    // setup display
    displayA.begin(SH1106_SWITCHCAPVCC);
    displayA.display();
    //displayB.begin(SH1106_SWITCHCAPVCC);
    //displayB.display();
    displayA.clearDisplay();
    displayA.setTextSize(2);
    displayA.setTextColor(WHITE);
    displayA.setCursor(0,0);
    displayA.println(F("Display A"));
    displayA.display();
    // displayB.clearDisplay();
    // displayB.setTextSize(2);
    // displayB.setTextColor(WHITE);
    // displayB.setCursor(0,0);
    // displayB.println(F("Display B"));
    // displayB.display();
    delay(1000);

    // joystick setup
    analogReference(DEFAULT);
    pinMode(JSTK_X, INPUT);
    pinMode(JSTK_Y, INPUT);
    pinMode(JSTK_SW, INPUT_PULLUP);

    EventTimer::Init();
    EventTimer::RegisterSource(&linearstageA);

    // init steppers
    linearstageA.init();
    //linearstageB.init();
    //pumpB.init();

    // enable both drivers
    digitalWrite(STEP_EN, LOW); // enable driver

    //linearstageA.calibrate();

    //delay(2000);
    injectorA.set_isotope(3);
    injectorA.set_activity(10.0f, now(), 0.6);

    linearstageA.home(LinearStage::DIR_NEG);
    linearstageA.move_abs(40, 6., 4.0, 100);
    //linearstageA.wait_move();
    //delay(5000);
    //linearstageA.search();
}

void loop() {
    displayA.clearDisplay();
    injectorA.update_display();

    // serial control
    // $name:length:data:checksum;
    // char '$'
    // char[2] name
    // char[2] cmd
    // uint8_t length 
    // char[255]
    // char ';'

    // Serial.parseInt()
    // Serial.parseFloat()

    // displayA.setCursor(0,56);
    // displayA.setTextSize(0);
    // displayA.print(year(), DEC);
    // displayA.print(F("."));
    // displayA.print(month(), DEC);
    // displayA.print(F("."));
    // displayA.print(day(), DEC);
    // displayA.print(F(" "));
    // displayA.print(hour(), DEC);
    // displayA.print(F(":"));
    // displayA.print(minute(), DEC);
    // displayA.print(F(":"));
    // displayA.print(second(), DEC);
    
    displayA.display();
    delay(25);
    //if(linearstageA.event_ready && linearstageA.event_time <= micros())
    //{
        //linearstageA.event_execute();
    //}
}