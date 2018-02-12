#pragma once

#include <Arduino.h>
#include <DS3234RTC.h>
#include <TimeLib.h>
#include <Time.h>
#include "Config.h"
#include "pins.h"
#include "LinearStage.h"
#include "EventTimer.h"
#include "Injector.h"

LinearStage linearstageA(STEPA_EN, STEPA_STEP, STEPA_CS, STEPA_STALL, 'A');
//LinearStage linearstageB(STEPB_EN, STEPB_STEP, STEPB_CS, STEPB_STALL, 'B');

//Adafruit_ST7735 display(DISP_CS, DISP_DC, DISP_RST);

Injector injectorA(&linearstageA);
//Injector injectorB(&linearstageA, &display);

enum LoopState {
    STATE_INIT,
    STATE_IDLE,
    STATE_HOMING,
    STATE_JOG_NEG,
    STATE_JOG_POS,
    STATE_SETUP
};

LoopState loop_state;