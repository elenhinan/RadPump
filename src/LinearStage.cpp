#include "LinearStage.h"

LinearStage *int0_cb_instance;
LinearStage *int1_cb_instance;
LinearStage *moveengine_stageA;
LinearStage *moveengine_stageB;

uint32_t *step_timer_time;

void int0_callback() { int0_cb_instance->stall_event(); }
void int1_callback() { int1_cb_instance->stall_event(); }

LinearStage::LinearStage(uint8_t pinEN, uint8_t pinSTEP, uint8_t pinCS, uint8_t pinDIAG1, char name) :
    pinEN(pinEN),
    pinSTEP(pinSTEP),
    pinCS(pinCS),
    pinSTALL(pinDIAG1),
    name(name)
{
    stepPort = portOutputRegister(digitalPinToPort(pinSTEP));
    stepMask = digitalPinToBitMask(pinSTEP);
    endstop = MM2STEP(LENGTH);
}

void LinearStage::init()
{
#ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(name);
    Serial.println(F(" initializing.."));
    Serial.print(F("  en: "));
    Serial.println(pinEN,DEC);
    Serial.print(F("  step: "));
    Serial.println(pinSTEP,DEC);
    Serial.print(F("  cs: "));
    Serial.println(pinCS,DEC);
    Serial.print(F("  stall: "));
    Serial.println(pinSTALL,DEC);
#endif
    pinMode(this->pinSTALL, INPUT);

    stepper_sgt = STALL_VALUE;

    this->stepper = new TMC2130Stepper(pinEN, -1, pinSTEP, pinCS);
    this->setup_driver();

    state = MOVE_STOP;
    stallguard_enabled = false;
    
    switch(digitalPinToInterrupt(pinSTALL))
    {
        case 0: Serial.println(F("  int0")); int0_cb_instance = this; attachInterrupt(0, int0_callback, RISING); break;
        case 1: Serial.println(F("  int1")); int1_cb_instance = this; attachInterrupt(1, int1_callback, RISING); break;
        default: Serial.println(F("  no interrupt")); break;
    }
    Serial.print(F("  steps/mm: "));
    Serial.println(STEPMM,DEC);
    Serial.println(F("  done!"));

    event_ready = false;
}

void LinearStage::stall_event()
{
    if(stallguard_enabled)
    {
         state = MOVE_STALLED;
    }
}

void LinearStage::setup_driver()
{
    //stepper->begin();
    // configure stepper driver
	stepper->rms_current(200); // mA
    stepper->microsteps(MICROSTEPS);
    stepper->interpolate(1);
    stepper->external_ref(0);
    stepper->internal_sense_R(0);
    //stepper->ihold(15); // standstill current 0-31/31
    //stepper->irun(31); // running current 0-31/31
    stepper->hold_delay(32);
    stepper->power_down_delay(255);
    
    stepper->random_off_time(0);
    stepper->disable_I_comparator(0);
    stepper->hysterisis_start(3);
    stepper->hysterisis_low(3);
    stepper->blank_time(36);
    stepper->off_time(5);
    //stepper->chopper_mode(0);
    stepper->diag1_stall(1);
    stepper->diag1_active_high(1);
    stepper->sg_stall_value(stepper_sgt);
    stepper->sg_filter(0);

    // stealthchop settings
    stepper->stealth_freq(1);
    //stepperA.stealth_autoscale(1);
    //stepperA.stealth_gradient(5);
    //stepperA.stealth_amplitude(255);
    //stepperA.sg_stall_value(0);


    // set auto switch from stealthstep
    //stepper->stealth_max_speed(MMS2TSTEP(20)); // disable stealthstep above this velocity
    //stepper->coolstep_min_speed(MMS2TSTEP(8.0)); // disable coolstep and stallguard below this velocity
    //stepper->mode_sw_speed(MMS2TSTEP(40));    // fullstep above this speed
    
    stealthchop(true);
}

void LinearStage::stallguard(bool enable)
{
    stallguard_enabled = enable;
    #ifdef DEBUG
    Serial.print(F("  stallguard:"));
    Serial.println(enable,DEC);
    #endif
}

void LinearStage::stealthchop(bool enable)
{
    stepper->stealthChop(enable);
    stepper->coolstep_min_speed(enable?0:uint16_t(FREQ2TSTEP(MIN_SD_SPEED)));
    #ifdef DEBUG
    Serial.print(F("  stealthchop:"));
    Serial.println(enable,DEC);
    #endif
}

void LinearStage::home(int8_t home_dir)
{
    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(name);
    Serial.println(F(" homing"));
    #endif
    stealthchop(false);
    stallguard(true);
    if(home_dir == DIR_BOTH || home_dir == DIR_POS)
    {
        move(LENGTH*1.10*STEPMM, 5.0*STEPMM, 50.0*STEPMM, false, 0); // move up to 110% of defined length
        wait_move(); // wait for stall (or end of move)

        if(state == MOVE_STALLED)
        {
            #ifdef DEBUG
            Serial.print(F("  endstop found: "));
            Serial.println(position);
            #endif
            endstop = position; // find endstop first
        }
    }
    if(home_dir == DIR_BOTH || home_dir == DIR_NEG)
    {
        move(-LENGTH*1.10*STEPMM, 5.0*STEPMM, 50.0*STEPMM, false, 0); // move up to 110% of defined length
        wait_move(); // wait until move done (because of a stall)

        if(state == MOVE_STALLED)
        {
            #ifdef DEBUG
            Serial.print(F("  home found: "));
            Serial.println(position);
            #endif
            if(home_dir == DIR_BOTH)
            {
                endstop -= position; // reposition endstop relative to new home
            }
            position = 0;
        }
    }
    #ifdef DEBUG
    Serial.print(F("  range: [0.00 (mm), "));
    Serial.print(float(endstop)/float(STEPMM));
    Serial.println(F(" (mm)]"));
    #endif

    stallguard(false);
    stealthchop(true);
}

void LinearStage::calibrate()
{
    // tune stallguard2
    bool done_tuning = false;
    // setup stepper driver
    //setup_driver();
    stepper->stealthChop(false);
    stepper->sg_filter(1);

    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(name);
    Serial.println(F(" calibrating.."));
    #endif
    int8_t sgt = 0; // start at zero according to datasheet
    stepper->sg_stall_value(sgt);
    uint16_t repetitions = 0;
    uint16_t steps = 0;
    uint16_t sg_value;
    uint16_t sg_value_max = 0;
    bool sgt_increase = true;
    bool sg_zero = true;
    const unsigned long tstep = SG2_TUNE_TSTEP; // us between steps
    unsigned long nexttime = micros();

    while(!done_tuning)
    {
        if(micros() <= nexttime)
            continue;
        nexttime += tstep;
        step();

        if(++steps == MICROSTEPS*5) // sg_filter filters last 4 fullsteps
        {
            steps = 0;
            sg_value = stepper->sg_result();
            sg_value_max = max(sg_value_max, sg_value);
            #ifdef DEBUG
            Serial.print(F("  SGT: "));
            Serial.print(sgt,DEC);
            Serial.print(F(", SG: "));
            Serial.println(sg_value,DEC);
            #endif
            if(sgt_increase)
            {
                if(sgt >= 63 || sg_value >= 1023)
                {
                    sgt_increase = false;
                    repetitions = 0;
                }
                else if(++repetitions == SG2_REP)
                {
                    stepper->sg_stall_value(++sgt);
                    repetitions = 0;
                }
            }
            else
            {
                sg_zero &= (sg_value == 0);
                if(++repetitions == SG2_REP)
                {
                    if(sg_zero)
                    {
                        done_tuning = true;
                    }
                    else
                    {
                        stepper->sg_stall_value(--sgt);
                        if(sgt == -64)
                            done_tuning = true;
                        repetitions = 0;
                        sg_zero = true;
                    }
                }
            }
        }
        
    }
    stepper_sgt = sgt;
    #ifdef DEBUG
    Serial.print(F("  Tuning done, optimal sgt: "));
    Serial.println(stepper_sgt,DEC);
    Serial.print(F("  Max SG value: "));
    Serial.println(sg_value_max,DEC);
    #endif

    stepper->sg_filter(0);
    stepper->stealthChop(true);
}

bool LinearStage::search()
{
    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(name);
    Serial.println(F(" search"));
    #endif

    stealthchop(false);
    stallguard(false);

    uint32_t sg_val_avg = 0;
    uint32_t sg_val_num = 0;
    //move_abs(70., 5.0, 5.0, 0); // move up to 20mm
    //wait_move();
    //move_abs(65., 5.0, 50.0, 0); // move up to 20mm
    //int32_t prev_position = position;
    // while(state != MOVE_STOP && state != MOVE_STALLED)
    // {
    //     if(prev_position - position < MICROSTEPS && state == MOVE_SLEW)
    //     {
    //         sg_val_avg += stepper->sg_result();
    //         sg_val_num++;
    //         prev_position = position;
    //     }
    // }
    // uint32_t sg_val_thr = sg_val_avg*100/(sg_val_num * 90); // threshold
    // #ifdef DEBUG
    // Serial.print(F("  threshold: "));
    // Serial.println(sg_val_thr,DEC);
    // delay(2000);
    // #endif
    move_abs(20, 1.0, 5.0, 0); // move up to 20mm
	stepper->rms_current(100); // mA

    while(state != MOVE_STOP && state != MOVE_STALLED)
    {
        Serial.println(stepper->sg_result(),DEC);
        delay(100);
    }

    #ifdef DEBUG
    Serial.print(F("  found: 0 ("));
    Serial.print(position,DEC);
    Serial.println(F(")"));
    #endif
    //position = 0;
    stealthchop(true);
    stallguard(false);
    return true;
}

void LinearStage::move_rel(float x, float dx, float ddx, uint32_t start_time)
{
    move(x*STEPMM + position, dx*STEPMM, ddx*STEPMM, true, start_time);
}

void LinearStage::move_abs(float x, float dx, float ddx, uint32_t start_time)
{
    move(x*STEPMM, dx*STEPMM, ddx*STEPMM, true, start_time);
}

void LinearStage::move(float x, float dx, float ddx, bool limit, uint32_t start_time)
{
    planner_init(x, dx, ddx, limit, start_time);
    event_ready = planner_advance();
    event_time = planner_next_time;
    EventTimer::Prime();
}

void LinearStage::event_execute()
{
    step();
    event_ready = planner_advance();
    event_time = planner_next_time;
}

void LinearStage::planner_init(float x, float dx, float ddx, bool limit, uint32_t start_time)
{
    planner_speed = dx*EventTimer::dt;
    planner_accel = ddx*EventTimer::dt*EventTimer::dt;
    planner_speed_inv = 1. / planner_speed;
    planner_accel_inv = 1. / (planner_accel * 0.5); // pre-calculate 0.5 i4. t² * 0.5*accel
    planner_target = (int32_t)(x+0.5);
    planner_target>position ? dir(DIR_POS) : dir(DIR_NEG); // set direction of movement

    #ifdef DEBUG
    Serial.print(F("Stage #"));
    Serial.print(name);
    Serial.println(F("  move"));
    Serial.print(F("  pos: "));
    Serial.print(x/float(STEPMM));
    Serial.print(F(" (mm)  speed: "));
    Serial.print(dx/float(STEPMM));
    Serial.print(F(" (mm/s)  accel: "));
    Serial.print(ddx/float(STEPMM));
    Serial.println(F(" (mm/s²)"));
    #endif

    if(limit && (planner_target < 0 || planner_target > endstop)) // check limits
    {
        planner_target = constrain(planner_target, 0, endstop);
        #ifdef DEBUG
        Serial.println(F("  WARNING out of range!"));
        Serial.print(F("  new target: "));
        Serial.print(planner_target*STEPMM);
        Serial.println(F(" (mm)"));
        #endif

    }

    //   Trapezoidal Speed Profile   //
    //            ________           //
    //           /|      |\          //
    //          / |      | \         //
    //       __/  |      |  \__      //
    //         |  |      |  |        //
    //         |tr|--ts--|tr|        //
    //         0  1      2  3        //

    planner_d0 = 0; // distance to start
    planner_d3 = abs(planner_target - position); // distance to end
    planner_d1 = planner_speed * planner_speed / (planner_accel*2); // distance to slew start

    if(2*planner_d1 < planner_d3) // enough space to accel and decel
    {
        planner_d2 = planner_d3 - planner_d1; // distance to slew end
    }
    else if (2*planner_d1 >= planner_d3) // not enough space, skip slewing
    {
        planner_d1 = planner_d3 / 2;
        planner_d2 = planner_d3 - planner_d1;
    }

    // use steps for each block to determine timing for each block
    planner_t0 = EventTimer::Now() + uint32_t((float)start_time*EventTimer::dt);
    planner_t1 = (uint32_t)(planner_t0 + sqrt(float(planner_d1)*planner_accel_inv)+0.5);
    planner_t2 = (uint32_t)(planner_t1 + float(planner_d2-planner_d1+1)*planner_speed_inv+0.5);
    planner_t3 = (uint32_t)(planner_t2 + sqrt(float(planner_d3-planner_d2-1)*planner_accel_inv)+0.5);
    
    // calculations for time to next step
    //         t0 = now
    // d<d1  d(t) = t*t*accel
    //       t(d) = sqrt(d/accel)
    //         t1 = t(d1)
    // d<d2  d(t) = d1 + (t-t1)*speed
    //       t(d) = t1 + (d-d1)/speed
    //         t2 = t(d2)
    // d<d3  d(t) = d2 + (t-t2)(speed-(t-t2)*accel)
    //       d(t) = d3 - (t3-t)(t3-t)*accel
    //       t(d) = t3 - sqrt((d3-d)/accel)
    
    state = MOVE_ACCEL;
    planner_step = 0;
}

bool LinearStage::planner_advance() {
    switch(state) // remove state and change to if's
    {
        case MOVE_ACCEL:    planner_next_time = planner_t0 + (uint32_t)(sqrt(float(planner_step)*planner_accel_inv)); break;
        case MOVE_SLEW:     planner_next_time = planner_t1 + (uint32_t)((float)(planner_step-planner_d1)*planner_speed_inv); break;
        case MOVE_DECEL:    planner_next_time = planner_t3 - (uint32_t)(sqrt((float)(planner_d3-planner_step)*planner_accel_inv)); break;
        case MOVE_STOP:     return false;
        case MOVE_STALLED:  return false;
    }

    if(planner_step==planner_d2) state = MOVE_DECEL; // check for d2 first, in case d1==d2 then skip slewing
    else if(planner_step==planner_d1) state = MOVE_SLEW;
    else if(planner_step==planner_d3) state = MOVE_STOP;
    
    planner_step++;
    return true;
}

void LinearStage::stop()
{
    state = MOVE_STOP;
    event_ready = false;
    event_time = 0xFFFFFFFF;
}

void LinearStage::wait_move()
{
    while(state != MOVE_STOP && state != MOVE_STALLED)
    {
        delayMicroseconds(10);
    }
    #ifdef DEBUG
    Serial.print(F("  stop: "));
    Serial.print(position/float(STEPMM));
    Serial.println(F(" mm"));
    #endif
}