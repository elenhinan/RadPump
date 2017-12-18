
#pragma once
#ifdef ARDUINO_AVR_UNO
//define SCK         13
//define MISO        12
//define MOSI        11
#define OLED_RST    7
#define OLED_DC     10
#define OLEDA_CS    9
#define OLEDB_CS    8
#define STEP_EN     7
#define STEPA_STEP  5
#define STEPA_CS    4
#define STEPA_STALL 3
#define STEPB_STEP  6
#define STEPB_CS    A3
#define STEPB_STALL 2
#define JSTK_X      A0
#define JSTK_Y      A1
#define JSTK_SW     A2
#endif

// arduino samd21
#ifdef ARDUINO_SAMD_ZERO
//define SCK         ICSP-3
//define MISO        ICSP-1
//define MOSI        ICSP-4
//define SCA         D21
//define SDA         D20
#define OLED_RST    13
#define OLED_DC     12
#define OLEDA_CS    11
#define OLEDB_CS    10
#define STEP_EN     9
#define STEPA_STEP  8
#define STEPA_CS    7
#define STEPA_STALL 6
#define STEPB_STEP  5
#define STEPB_CS    4
#define STEPB_STALL 3
//#define STEPC_STEP  2
//#define STEPC_CS    1
//#define STEPC_STALL 0
#define JSTK_X      A0
#define JSTK_Y      A1
#define JSTK_SW     A2
#endif