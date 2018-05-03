
#pragma once
#ifdef ARDUINO_GENERIC_STM32F103C

// unused               PB11
// unused               PB10
// unused               PB1
// unused               PB0
#define DISP_MOSI       PA7
#define DISP_MISO       PA6
#define DISP_SCK        PA5
#define DISP_RST        PA4
#define DISP_DC         PA3
#define DISP_CS         PA2
#define BUTTON_A_POS    PA1
#define BUTTON_A_NEG    PA0
#define BUTTON_B_POS    PC15
#define BUTTON_B_NEG    PC14
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
#define STEPB_STEP      PA15
#define STEPB_CS        PB3
#define STEPB_STALL     PB4
#define CTRL_RST        PB5
#define CTRL_TX         PB6 // TX1
#define CTRL_RX         PB7 // RX1
#define STEPA_EN        PB8
#define STEPB_EN        PB9

#endif