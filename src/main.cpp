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

LinearStage linearstageA(STEP_EN, STEPA_STEP, STEPA_CS, STEPA_STALL);
//LinearStage linearstageB(STEP_EN, STEPB_DIR, STEPB_STEP, STEPB_CS, STEPB_STALL);

Adafruit_SH1106 displayA(OLED_DC, OLED_RST, OLEDA_CS);
Adafruit_SH1106 displayB(OLED_DC, OLED_RST, OLEDB_CS);

Injector injectorA(&linearstageA, &displayA);
//Injector injectorB(linearstageB, displayB);

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
    EventTimer::RegisterSource(&linearstageA);

    // init steppers
    linearstageA.init();
    //pumpB.init();

    // enable both drivers
    digitalWrite(STEP_EN, LOW); // enable driver

    //linearstageA.calibrate();
    //linearstageA.home(LinearStage::DIR_BOTH);
    //linearstageA.dir(LinearStage::DIR_NEG);
    delay(2000);
    //linearstageA.search();
    linearstageA.home(LinearStage::DIR_NEG);
    linearstageA.move_abs(40, 6., 4.0, 100);
    linearstageA.wait_move();
    //delay(5000);
    //linearstageA.search();
}

void loop() {
    displayA.clearDisplay();
    injectorA.update_display();
    displayA.display();
    delay(25);
    //if(linearstageA.event_ready && linearstageA.event_time <= micros())
    //{
        //linearstageA.event_execute();
    //}
}