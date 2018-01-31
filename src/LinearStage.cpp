#include "LinearStage.h"

#define VERIFY2(FUNCTION, VALUE) stepper.FUNCTION(VALUE); SERIAL_DEBUG.print( #FUNCTION "(" #VALUE ") "); SERIAL_DEBUG.println(stepper.FUNCTION());
#define VERIFY(FUNCTION, VALUE) VERIFY2(FUNCTION, VALUE)

LinearStage::LinearStage(uint8_t pinEN, uint8_t pinSTEP, uint8_t pinCS, uint8_t pinDIAG1, char name) :
    stepper(pinEN, -1, pinSTEP, pinCS),
    pinEN(pinEN),
    pinSTEP(pinSTEP),
    pinCS(pinCS),
    pinSTALL(pinDIAG1),
    name(name)
{
    stepPort = digitalPinToPort(pinSTEP);
    stepMask = digitalPinToBitMask(pinSTEP);
    endstop = MM2STEP(LENGTH);
    pinMode(pinSTALL, INPUT);
}

void LinearStage::step()
{
    stepPort->regs->ODR ^= stepMask;
    //stepPort->regs->BSRR = stepMask;
    //stepPort->regs->BRR  = stepMask;
    position += direction;
}

void LinearStage::init()
{
#ifdef DEBUG
    SERIAL_DEBUG.print(F("Stage #"));
    SERIAL_DEBUG.print(name);
    SERIAL_DEBUG.println(F(" initializing.."));
    SERIAL_DEBUG.print(F("  en: "));
    SERIAL_DEBUG.println(pinEN,DEC);
    SERIAL_DEBUG.print(F("  step: "));
    SERIAL_DEBUG.println(pinSTEP,DEC);
    SERIAL_DEBUG.print(F("  cs: "));
    SERIAL_DEBUG.println(pinCS,DEC);
    SERIAL_DEBUG.print(F("  stall: "));
    SERIAL_DEBUG.println(pinSTALL,DEC);
    SERIAL_DEBUG.print(F("  steps/mm: "));
    SERIAL_DEBUG.println(STEPMM,DEC);
    SERIAL_DEBUG.println(F("  done!"));
#endif

    // initialize stepper driver()
    stepper.begin();
    setup_driver();

    // initial state
    state = MOVE_STOP;
    stallguard_enabled = false;
    event_ready = false;
    
    // setup stall guard interrupt
    attachInterrupt(pinSTALL, LinearStage::stall_event, this, RISING);
}

void LinearStage::stall_event(void* linearstage_ptr)
{
    LinearStage* ls = (LinearStage*)linearstage_ptr;
    if(ls->stallguard_enabled && ls->state == MOVE_SLEW)
    {
         ls->state = MOVE_STALLED;
    }
}

void LinearStage::setup_driver()
{
    stepper_sgt = STALL_VALUE;
    //stepper.begin();
    // configure stepper driver
    VERIFY(rms_current, STEPPERCURRENT); // mA
    VERIFY(microsteps , MICROSTEPS);
    //while(stepper.interpolate() != 1)
        //stepper.interpolate(1);
    VERIFY(interpolate, 1)
    VERIFY(external_ref, 0);
    VERIFY(internal_sense_R, 0);
    //stepper.ihold(15); // standstill current 0-31/31
    //stepper.irun(31); // running current 0-31/31
    VERIFY(hold_delay, 32);
    VERIFY(power_down_delay, 255);
    VERIFY(dedge, true); // enable trigger on falling and rising edge
    VERIFY(random_off_time, 0);
    VERIFY(disable_I_comparator, 0);
    VERIFY(hysterisis_start, 3);
    VERIFY(hysterisis_low, 3);
    VERIFY(blank_time, 36);
    VERIFY(off_time, 5);
    VERIFY(diag1_stall, 1);
    VERIFY(diag1_active_high, 1);
    VERIFY(sg_stall_value, STALL_VALUE);
    VERIFY(sg_filter, 0);

    // stealthchop settings
    stepper.stealth_freq(1);
    //stepperA.stealth_autoscale(1);
    //stepperA.stealth_gradient(5);
    //stepperA.stealth_amplitude(255);
    //stepperA.sg_stall_value(0);

    // set auto switch from stealthstep
    //stepper.stealth_max_speed(MMS2TSTEP(20)); // disable stealthstep above this velocity
    //stepper.coolstep_min_speed(MMS2TSTEP(8.0)); // disable coolstep and stallguard below this velocity
    //stepper.mode_sw_speed(MMS2TSTEP(40));    // fullstep above this speed
    
    stealthchop(true);
}

void LinearStage::stallguard(bool enable)
{
    stallguard_enabled = enable;
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("  stallguard:"));
    SERIAL_DEBUG.println(enable,DEC);
    #endif
}

void LinearStage::stealthchop(bool enable)
{
    stepper.stealthChop(enable);
    stepper.coolstep_min_speed(enable?0:uint16_t(FREQ2TSTEP(MIN_SD_SPEED)));
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("  stealthchop:"));
    SERIAL_DEBUG.println(enable,DEC);
    #endif
    digitalWrite(LED_BUILTIN, enable);
}

void LinearStage::home(int8_t home_dir)
{
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("Stage #"));
    SERIAL_DEBUG.print(name);
    SERIAL_DEBUG.println(F(" homing"));
    #endif
    stealthchop(false);
    stallguard(true);
    if(home_dir == DIR_BOTH || home_dir == DIR_POS)
    {
        move(LENGTH*1.10*STEPMM, HOMING_SPEED*STEPMM, HOMING_ACCEL*STEPMM, false, 0); // move up to 110% of defined length
        wait_move(); // wait for stall (or end of move)

        if(state == MOVE_STALLED || state == MOVE_STOP)
        {
            #ifdef DEBUG
            SERIAL_DEBUG.print(F("  endstop found: "));
            SERIAL_DEBUG.println(position);
            #endif
            endstop = position; // find endstop first
        }
    }
    if(home_dir == DIR_BOTH || home_dir == DIR_NEG)
    {
        move(-LENGTH*1.10*STEPMM, 5.0*STEPMM, 50.0*STEPMM, false, 0); // move up to 110% of defined length
        wait_move(); // wait until move done (because of a stall)

        if(state == MOVE_STALLED || state == MOVE_STOP)
        {
            #ifdef DEBUG
            SERIAL_DEBUG.print(F("  home found: "));
            SERIAL_DEBUG.println(position);
            #endif
            if(home_dir == DIR_BOTH)
            {
                endstop -= position; // reposition endstop relative to new home
            }
            position = 0;
        }
    }
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("  range: [0.00 (mm), "));
    SERIAL_DEBUG.print(float(endstop)/float(STEPMM));
    SERIAL_DEBUG.println(F(" (mm)]"));
    #endif
}

void LinearStage::calibrate()
{
    // tune stallguard2
    bool done_tuning = false;
    // setup stepper driver
    //setup_driver();
    stallguard(false);
    stealthchop(false);

    stepper.sg_filter(1);

    #ifdef DEBUG
    SERIAL_DEBUG.print(F("Stage #"));
    SERIAL_DEBUG.print(name);
    SERIAL_DEBUG.println(F(" calibrating.."));
    #endif
    int8_t sgt = 0; // start at zero according to datasheet
    stepper.sg_stall_value(sgt);
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
            sg_value = stepper.sg_result();
            sg_value_max = max(sg_value_max, sg_value);
            #ifdef DEBUG
            SERIAL_DEBUG.print(F("  SGT: "));
            SERIAL_DEBUG.print(sgt,DEC);
            SERIAL_DEBUG.print(F(", SG: "));
            SERIAL_DEBUG.println(sg_value,DEC);
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
                    stepper.sg_stall_value(++sgt);
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
                        stepper.sg_stall_value(--sgt);
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
    SERIAL_DEBUG.print(F("  Tuning done, optimal sgt: "));
    SERIAL_DEBUG.println(stepper_sgt,DEC);
    SERIAL_DEBUG.print(F("  Max SG value: "));
    SERIAL_DEBUG.println(sg_value_max,DEC);
    #endif

    stepper.sg_filter(0);
    stepper.stealthChop(true);
}

bool LinearStage::search()
{
    #ifdef DEBUG
    SERIAL_DEBUG.print(F("Stage #"));
    SERIAL_DEBUG.print(name);
    SERIAL_DEBUG.println(F(" search"));
    #endif

    stealthchop(false);
    stallguard(false);

    //uint32_t sg_val_avg = 0;
    //uint32_t sg_val_num = 0;
    //move_abs(70., 5.0, 5.0, 0); // move up to 20mm
    //wait_move();
    //move_abs(65., 5.0, 50.0, 0); // move up to 20mm
    //int32_t prev_position = position;
    // while(state != MOVE_STOP && state != MOVE_STALLED)
    // {
    //     if(prev_position - position < MICROSTEPS && state == MOVE_SLEW)
    //     {
    //         sg_val_avg += stepper.sg_result();
    //         sg_val_num++;
    //         prev_position = position;
    //     }
    // }
    // uint32_t sg_val_thr = sg_val_avg*100/(sg_val_num * 90); // threshold
    // #ifdef DEBUG
    // SERIAL_DEBUG.print(F("  threshold: "));
    // SERIAL_DEBUG.println(sg_val_thr,DEC);
    // delay(2000);
    // #endif
    move_abs(20, 1.0, 5.0, 0); // move up to 20mm
	//stepper.rms_current(100); // mA

    while(state != MOVE_STOP && state != MOVE_STALLED)
    {
        SERIAL_DEBUG.println(stepper.sg_result(),DEC);
        delay(10);
    }

    #ifdef DEBUG
    SERIAL_DEBUG.print(F("  found: 0 ("));
    SERIAL_DEBUG.print(position,DEC);
    SERIAL_DEBUG.println(F(")"));
    #endif
    //position = 0;
    stealthchop(true);
    stallguard(false);
    return true;
}

void LinearStage::move_rel(float x, float dx, float ddx, uint32_t start_time)
{
    stallguard(false);
    stealthchop(true);
    move(x*STEPMM + position, dx*STEPMM, ddx*STEPMM, true, start_time);
}

void LinearStage::move_abs(float x, float dx, float ddx, uint32_t start_time)
{
    stallguard(false);
    stealthchop(true);
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
    SERIAL_DEBUG.print(F("Stage #"));
    SERIAL_DEBUG.print(name);
    SERIAL_DEBUG.println(F("  move"));
    SERIAL_DEBUG.print(F("  pos: "));
    SERIAL_DEBUG.print(x/float(STEPMM));
    SERIAL_DEBUG.print(F(" (mm)  speed: "));
    SERIAL_DEBUG.print(dx/float(STEPMM));
    SERIAL_DEBUG.print(F(" (mm/s)  accel: "));
    SERIAL_DEBUG.print(ddx/float(STEPMM));
    SERIAL_DEBUG.println(F(" (mm/s²)"));
    #endif

    if(limit && (planner_target < 0 || planner_target > endstop)) // check limits
    {
        planner_target = constrain(planner_target, 0, endstop);
        #ifdef DEBUG
        SERIAL_DEBUG.println(F("  WARNING out of range!"));
        SERIAL_DEBUG.print(F("  new target: "));
        SERIAL_DEBUG.print(planner_target*STEPMM);
        SERIAL_DEBUG.println(F(" (mm)"));
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
    SERIAL_DEBUG.print(F("  stop: "));
    SERIAL_DEBUG.print(position/float(STEPMM));
    SERIAL_DEBUG.println(F(" mm"));
    #endif
}