#define DEBUG

// serial config
#define SERIAL_DEBUG Serial3

// stepper configuration
#define STEPPERCURRENT 200
#define STALL_VALUE 16 // [-64..63] default: 21
#define MICROSTEPS 8

// linear stage 
#define LENGTH 77.5f // ~78mm length
#define THREAD_PITCH 2.0f // 2mm/rev
#define STEP_DEG 1.8f // deg per step
#define SG2_TUNE_RPM 5 // RPM for autotuning stallguard2
#define SG2_REP 8 // repetitions per SGT value
#define HOMING_SPEED 5.0 // mm/s
#define HOMING_ACCEL 20.0 // mm/s²
#define MIN_SD_SPEED 1 // mm/s

// max isotopes and syringes stored in eeprom
#define ISOTOPE_MAX 16
#define SYRINGE_MAX 16

// colors
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

// drawing colors
#define SYRINGE_COLOR WHITE
#define LIQUID_COLOR CYAN
