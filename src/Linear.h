#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <TMC2130Stepper.h>

#define DEBUG

// config
#define LENGTH 80 // ~78mm length
#define STALL_VALUE 21 // [-64..63]
#define MICROSTEPS 16
#define THREAD_PITCH 2.0 // 2mm/rev
#define STEP_DEG 1.8 // deg per step
#define SG2_TUNE_RPM 5 // RPM for autotuning stallguard2
#define SG2_REP 8 // repetitions per SGT value
#define HOMING_SPEED 100
#define TMC2130_FLCK 13000000 // TMC2130 internal fclk

// macros
#define MM2STEP(x) (x / THREAD_PITCH) * (360 / STEP_DEG) * MICROSTEPS // mm/s to freq (fullstep)
#define FREQ2TSTEP(x) TMC2130_FLCK / (x * 256) // fullsteps/s to clock cycles
#define MMS2TSTEP(x) FREQ2TSTEP(MM2STEP(x)) // mm/s to tstep value
#define MMSTEP (1.0/MM2STEP(1))
#define STEPMM MM2STEP(1)
#define SG2_TUNE_TSTEP (60ul * 1000ul * 1000ul * STEP_DEG / (SG2_TUNE_RPM * MICROSTEPS * 360ul))

inline uint32_t sqrt32(uint32_t n) 
{ 
    uint32_t c = 0x8000; 
    uint32_t g = 0x8000; 
    for(;;) { 
        if(g*g > n)
            g ^= c; 
        c >>= 1; 
        if(c == 0)
            return g; 
        g |= c; 
    } 
}

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

static uint8_t LinearStageNumber = 0;

class LinearStage
{
private:
    // hw stuff
    TMC2130Stepper* stepper;
    bool stallguard_enabled;
    uint8_t pinEN;
    uint8_t pinDIR;
    uint8_t pinSTEP;
    uint8_t pinCS;
    uint8_t pinSTALL;
    uint8_t number;
    PortReg *stepPort;
    PortMask stepMask;
    PortReg *dirPort;
    PortMask dirMask;

    // driver settings
    int8_t stepper_sgt;

    // movement
    int8_t direction;
    int32_t position;
    int32_t endstop;
    int32_t target;
    volatile uint8_t state;
    
    volatile bool stalled;

    void setup_driver();
    void move(float x, float dx, float ddx, bool limit);
    static void setup_timer();

public:
    static const int8_t DIR_POS = 1;
    static const int8_t DIR_BOTH = 0;
    static const int8_t DIR_NEG = -1;
    static const uint8_t MOVE_STOP = 0;
    static const uint8_t MOVE_ACCEL = 1;
    static const uint8_t MOVE_SLEW = 2;
    static const uint8_t MOVE_DECEL = 3;
    static const uint8_t MOVE_STALLED = 4;
    static const uint8_t CHOPPER_STEALTH = 0;
    static const uint8_t CHOPPER_SPREAD = 1;
    static const uint8_t CHOPPER_STALLGUARD = 2;
    static const uint8_t CHOPPER_AUTO = 3;

    LinearStage(uint8_t pinEN, uint8_t pinDIR, uint8_t pinSTEP, uint8_t pinCS, uint8_t pinDIAG1);
    void init();
    void stall_event();
    inline void step() { if (!stalled) { *stepPort |=  stepMask; *stepPort &= ~stepMask; position += direction; } }
    inline void dir(int8_t direction) { this->direction = direction; direction == DIR_POS ? *dirPort |=  dirMask : *dirPort &= ~dirMask; }
    inline void reverse() { direction = -direction; *dirPort ^= dirMask; }
    inline uint16_t get_SG() { return stepper->sg_result(); }
    void stallguard(bool enable);
    void stealthchop(bool enable);
    void home(int8_t home_dir);
    void calibrate();
    bool search();
    inline int32_t get_home() { return 0; }
    inline int32_t get_endstop() { return endstop; }
    inline int32_t get_position() { return position; }
    void move_abs(float x, float dx, float ddx);
    void move_rel(float x, float dx, float ddx);
    void wait_move();
};