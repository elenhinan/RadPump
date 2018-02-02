
#pragma once
#ifdef ARDUINO_GENERIC_STM32F103C

#define CTRL_RX         PB11
#define CTRL_TX         PB10
#define BUTTON_POS      PB1
#define BUTTON_NEG      PB0
#define DISP_MOSI       PA7
#define DISP_MISO       PA6
#define DISP_SCK        PA5
// unused               PA4
#define DISP_RST        PA3
#define DISP_DC         PA2
#define DISP_CS         PA1
#define DISP_BL         PA0
// unused (limited)     PC15
// unused (limited)     PC14
// Built-in LED         PC13

// unused               PB12
#define STEPPER_SCK     PB13
#define STEPPER_MISO    PB14
#define STEPPER_MOSI    PB15
#define STEPA_STEP      PA8
#define STEPA_CS        PA9
#define STEPA_STALL     PA10
// USB-                 PA11
// USB+                 PA12
// unused               PA15
#define STEPB_STEP      PB3
#define STEPB_CS        PB4
#define STEPB_STALL     PB5
// unused               PB6
// unused               PB7
#define STEPA_EN        PB8
#define STEPB_EN        PB9

#endif