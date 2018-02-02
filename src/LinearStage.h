#pragma once

#include <Arduino.h>
//#include <Wire.h>
//#include <SPI.h>
#include <TMC2130Stepper.h>
#include "EventTimer.h"
#include "Config.h"

// config
#define TMC2130_FLCK 12000000 // TMC2130 internal fclk

// macros
#define MM2STEP(x) (x / THREAD_PITCH) * (360 / STEP_DEG) * MICROSTEPS // mm/s to freq
#define FREQ2TSTEP(x) TMC2130_FLCK / (x * (256/MICROSTEPS)) // clock cycles per 256 microstep
#define MMS2TSTEP(x) FREQ2TSTEP(MM2STEP(x)) // mm/s to tstep value
#define MMSTEP (1.0/MM2STEP(1))
#define STEPMM MM2STEP(1)
#define SG2_TUNE_TSTEP (60ul * 1000ul * 1000ul * STEP_DEG / (SG2_TUNE_RPM * MICROSTEPS * 360ul))

#ifdef ARDUINO_ARCH_AVR
    #define REGTYPE uint8_t
#else
    #define REGTYPE uint32_t
#endif

typedef volatile REGTYPE PortReg;
typedef REGTYPE PortMask;

class LinearStage : public TimedEvent
{
private:
    // hw stuff
    TMC2130Stepper stepper;
    bool stallguard_enabled;
    const uint8_t pinEN;
    const uint8_t pinSTEP;
    const uint8_t pinCS;
    const uint8_t pinSTALL;
    const char name;
    gpio_dev* stepPort;
    uint16_t stepMask;

    // driver settings
    int8_t stepper_sgt;

    // movement
    int8_t direction;
    int32_t position;
    int32_t endstop;
    uint8_t state;

    // movement planner
    int32_t planner_target;
    float planner_speed;
    float planner_accel;
    float planner_speed_inv;
    float planner_accel_inv;
    uint32_t planner_d0, planner_d1, planner_d2, planner_d3; // step # for ramp start, slew start, slew end, ramp end
    uint64_t planner_t0, planner_t1, planner_t2, planner_t3; // as above, but time expressed in units of ts
    void planner_init(float x, float dx, float ddx, bool limit, uint64_t start_time);
    bool planner_advance();

    // other functions
    void setup_driver();
    void move(float x, float dx, float ddx, bool limit, uint64_t start_time = 0);

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

    LinearStage(uint8_t pinEN, uint8_t pinSTEP, uint8_t pinCS, uint8_t pinDIAG1, char name);
    void init();
    static void stall_event(void* linearstage_ptr);
 
    void step();
    void dir(int8_t direction) { this->direction = direction; stepper.shaft_dir(direction == DIR_POS); }
    inline uint16_t get_sg() { return stepper.sg_result(); }
    void stallguard(bool enable);
    void stealthchop(bool enable);
    void home(int8_t home_dir);
    void calibrate();
    bool search();
    void stop();
    void enable() { digitalWrite(pinEN, LOW); }
    void disable() { digitalWrite(pinEN, HIGH); }
    int32_t get_home() { return 0; }
    int32_t get_endstop_steps() { return endstop; }
    int32_t get_position_steps() { return position; }
    float get_endstop_mm() { return float(endstop)*(1./float(STEPMM)); }
    float get_position_mm() { return float(position)*(1./float(STEPMM)); }
    
    void move_abs(float x, float dx, float ddx, uint64_t start_time = 0);
    void move_rel(float x, float dx, float ddx, uint64_t start_time = 0);
    void wait_move();
    void event_execute();
};