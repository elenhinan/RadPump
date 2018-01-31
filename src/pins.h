
#pragma once
#ifdef ARDUINO_GENERIC_STM32F103C
//      SCK         PA5
//      MISO        PA6
//      MOSI        PA7
//      RX          PB7
//      TX          PB6

#define DISP_RST    PA3
#define DISP_DC     PA2
#define DISP_CS     PA1
#define DISP_BL     PA0

#define STEPA_EN    PB8
#define STEPA_STEP  PA8
#define STEPA_CS    PA9
#define STEPA_STALL PA10
#define STEPB_EN    PB9
#define STEPB_STEP  PB3
#define STEPB_CS    PB4
#define STEPB_STALL PB5
#define JSTK_X      PB1
#define JSTK_Y      PB0
#define JSTK_SW     PB10
#endif