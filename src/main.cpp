#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
//#include <TMC2130Stepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "pins.h"
#include "LinearStage.h"
#include "EventTimer.h"
#include "Injector.h"

#define BAUDRATE 115200

LinearStage pumpA(STEP_EN, STEPA_DIR, STEPA_STEP, STEPA_CS, STEPA_STALL);
//LinearStage pumpB(STEP_EN, STEPB_DIR, STEPB_STEP, STEPB_CS, STEPB_STALL);

Adafruit_SH1106 displayA(OLED_DC, OLED_RST, OLEDA_CS);
Adafruit_SH1106 displayB(OLED_DC, OLED_RST, OLEDB_CS);

void setup() {
    // disable both drivers before setup
    pinMode(STEP_EN, OUTPUT);
    digitalWrite(STEP_EN, HIGH); // disable drivers

    // setup serial
    Serial.begin(BAUDRATE);

    // setup display
    displayA.begin(SH1106_SWITCHCAPVCC);
    displayA.display();
    displayB.begin(SH1106_SWITCHCAPVCC);
    displayB.display();
    displayA.clearDisplay();
    displayA.setTextSize(2);
    displayA.setTextColor(WHITE);
    displayA.setCursor(0,0);
    displayA.println("Display A");
    displayA.display();
    displayB.clearDisplay();
    displayB.setTextSize(2);
    displayB.setTextColor(WHITE);
    displayB.setCursor(0,0);
    displayB.println("Display B");
    displayB.display();
    delay(1000);

    // joystick setup
    analogReference(DEFAULT);
    pinMode(JSTK_X, INPUT);
    pinMode(JSTK_Y, INPUT);
    pinMode(JSTK_SW, INPUT_PULLUP);

    EventTimer::Init();
    EventTimer::RegisterSource(&pumpA);

    // init steppers
    pumpA.init();
    //pumpB.init();

    // enable both drivers
    digitalWrite(STEP_EN, LOW); // enable driver

    //pumpA.calibrate();
    //pumpA.home(LinearStage::DIR_BOTH);
    //pumpA.dir(LinearStage::DIR_NEG);
    delay(2000);
    //pumpA.search();
    pumpA.home(LinearStage::DIR_BOTH);
    pumpA.move_abs(40,8.,20.0, 100);
}

void loop() {
    displayA.clearDisplay();
    displayA.setTextSize(1);
    displayA.setTextColor(WHITE);
    displayA.setCursor(0,32);

    displayA.print(F("pos: "));
    displayA.println(pumpA.get_position(),DEC);
    displayA.display();
    delay(25);
    //if(pumpA.event_ready && pumpA.event_time <= micros())
    //{
        //pumpA.event_execute();
    //}
}