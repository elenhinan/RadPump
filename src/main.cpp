#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
//#include <TMC2130Stepper.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "Linear.h"

#define BAUDRATE 115200

#ifdef ARDUINO_AVR_UNO
//define SCK         13
//define MISO        12
//define MOSI        11
#define OLED_RST    7
#define OLED_DC     10
#define OLEDA_CS    9
#define OLEDB_CS    8
#define STEP_EN     7
#define STEPA_DIR   6
#define STEPA_STEP  5
#define STEPA_CS    4
#define STEPA_STALL 3
#define STEPB_DIR   A3
#define STEPB_STEP  A4
#define STEPB_CS    A5
#define STEPB_STALL 2
#define JSTK_X      A0
#define JSTK_Y      A1
#define JSTK_SW     A2
#endif

//TMC2130Stepper stepperA(STEP_EN, STEPA_DIR, STEPA_STEP, STEPA_CS);
//TMC2130Stepper stepperB(STEP_EN, STEPB_DIR, STEPB_STEP, STEPB_CS);
LinearStage pumpA(STEP_EN, STEPA_DIR, STEPA_STEP, STEPA_CS, STEPA_STALL);
Adafruit_SH1106 displayA(OLED_DC, OLED_RST, OLEDA_CS);
Adafruit_SH1106 displayB(OLED_DC, OLED_RST, OLEDB_CS);

void setup() {
    // disable both drivers before setup
    pinMode(STEP_EN, OUTPUT);
    digitalWrite(STEP_EN, HIGH); // disable drivers

    // setup serial
    //pinMode(OLEDA_CS, OUTPUT);
    //pinMode(OLEDB_CS, OUTPUT);
    //digitalWrite(OLEDA_CS, HIGH);
    //digitalWrite(OLEDB_CS, HIGH);
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

    // init steppers
    pumpA.init();

    // enable both drivers
    digitalWrite(STEP_EN, LOW); // enable driver

    //pumpA.calibrate();
    pumpA.stealthchop(false);
    pumpA.home(LinearStage::DIR_BOTH);
    pumpA.stealthchop(true);
    pumpA.dir(LinearStage::DIR_NEG);
    delay(2000);
}

void loop() {
    unsigned long nowtime = micros();
    static unsigned long lasttime = micros();
    static unsigned char counts = 0;    
    static bool done = false;
    // make a step
    //delay(100);
    // int x = analogRead(JSTK_X);
    // int y = analogRead(JSTK_Y);
    // displayA.clearDisplay();
    // displayA.setTextSize(2);
    // displayA.setCursor(0,0);
    // if (digitalRead(JSTK_SW)) {
    //     displayA.setTextColor(BLACK, WHITE); // 'inverted' text
    // } else {
    //     displayA.setTextColor(WHITE);    
    // }
    // displayA.println(x);
    // displayA.display();

    // displayB.clearDisplay();
    // displayB.setTextSize(2);
    // displayB.setCursor(0,0);
    // if (digitalRead(JSTK_SW)) {
    //     displayB.setTextColor(BLACK, WHITE); // 'inverted' text
    // } else {
    //     displayB.setTextColor(WHITE);    
    // }
    // displayB.println(y);
    // displayB.display();
    
    
    delayMicroseconds(200);
    if(pumpA.get_position() != (pumpA.get_endstop())/2)
    {
        pumpA.step();
    }
    else if(!done)
    {
        Serial.print("Position: ");
        Serial.println(pumpA.get_position());
        done = true;
    }
}